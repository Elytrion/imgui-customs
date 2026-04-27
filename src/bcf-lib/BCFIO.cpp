#include "BCFIO.h"
#include "XMLLib/XMLParser.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include "zip.h"
#include "stb_image.h"
#include "stb_image_write.h"

namespace fs = std::filesystem;
namespace
{
	// taken from https://github.com/buildingSMART/BCF-XML/tree/release_3_0/Documentation
	constexpr const char* versionFileName = "bcf.version";
	constexpr const char* documentsFileName = "documents.xml";
	constexpr const char* documentsFolderName = "Documents";
	constexpr const char* extensionsFileName = "extensions.xml";
	constexpr const char* projectFileName = "project.bcfp";
	constexpr const char* markupFileName = "markup.bcf";
	constexpr const char* visualExt = ".bcfv";
	constexpr const char* validSnapshotExt[3] = { ".png",".jpg",".jpeg" };

	static bool IsSnapshotFile(const fs::path& path)
	{
		const std::string ext = path.extension().string();
		auto lower = [](std::string s)
			{
				for (char& c : s)
					c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
				return s;
			};

		const std::string e = lower(ext);
		for (const char* validExt : validSnapshotExt)
		{
			if (e == validExt)
				return true;
		}
		return false;
	}

	// ---------------- READ HELPERS ---------------- 
	// expects zip to be open for reading
	bool ZipEntryExists(zip_t* zip, const std::string& name)
	{
		if (!zip)
			return false;

		int total = zip_entries_total(zip);

		for (int i = 0; i < total; ++i)
		{
			if (zip_entry_openbyindex(zip, i) != 0)
				continue;

			const char* entryName = zip_entry_name(zip);

			bool match = (entryName && name == entryName);

			zip_entry_close(zip);

			if (match)
				return true;
		}

		return false;
	}
	// expects zip to be open for reading
	std::optional<std::string> ReadXMLEntry(zip_t* zip, const std::string& name)
	{
		if (!zip)
			return std::nullopt;

		if (zip_entry_open(zip, name.c_str()) != 0)
			return std::nullopt;

		void* buf = nullptr;
		size_t size = 0;

		if (zip_entry_read(zip, &buf, &size) < 0 || size == 0)
		{
			zip_entry_close(zip);
			return std::nullopt;
		}

		std::string result((char*)buf, size);

		free(buf);
		zip_entry_close(zip);

		return result;
	}

	struct ZipGuard
	{
		zip_t* zip = nullptr;

		explicit ZipGuard(const std::string& path, char format, int compression = 0)
		{
			zip = zip_open(path.c_str(), compression, format);
		}
		~ZipGuard()
		{
			if (zip)
				zip_close(zip);
		}

		bool valid() const { return zip != nullptr; }
	};

	std::optional<XMLLib::XMLDocumentHandle> ParseXMLEntry(zip_t* zip, const std::string& name, std::string& errOut)
	{
		if (!ZipEntryExists(zip, name)) // check if entry exists in archive
		{
			errOut = "[ParseXMLEntry] Invalid format: Could not find " + name;
			return std::nullopt;
		}
		auto versOpt = ReadXMLEntry(zip, name); // attempt to read text from file
		if (!versOpt)
		{
			errOut = "[ParseXMLEntry] Invalid files: Could not open and read " + name;
			return std::nullopt;
		}
		auto versDocHandle = XMLLib::XMLParser::ParseMemory(versionFileName, versOpt.value()); // Attempt to parse xml into DOMdocument format
		if (!versDocHandle.IsValid())
		{
			errOut = "[ParseXMLEntry] Invalid Files: Could not DOM parse xml from " + name;
			return std::nullopt;
		}
		return versDocHandle;
	}

	struct TopicPathParts
	{
		std::string guid;
		std::string localName;
	};
	std::optional<TopicPathParts> SplitTopicEntryPath(const std::string& entryPath)
	{
		const size_t slashPos = entryPath.find('/');
		if (slashPos == std::string::npos)
			return std::nullopt;

		if (slashPos == 0 || slashPos + 1 >= entryPath.size())
			return std::nullopt;

		TopicPathParts parts;
		parts.guid = entryPath.substr(0, slashPos);
		parts.localName = entryPath.substr(slashPos + 1);

		return parts;
	}

	bool ZipHasFolder(zip_t* zip, const std::string& folderName)
	{
		if (!zip)
			return false;

		std::string prefix = folderName + "/";

		int total = zip_entries_total(zip);

		for (int i = 0; i < total; ++i)
		{
			if (zip_entry_openbyindex(zip, i) != 0)
				continue;

			const char* name = zip_entry_name(zip);
			std::string entry = name ? name : "";

			zip_entry_close(zip);

			if (entry.starts_with(prefix))
				return true;
		}

		return false;
	}

	bool ExtractZipEntryToFile(zip_t* zip,
		const std::string& entryName,
		const fs::path& workingRoot,
		std::string& errMsg)
	{
		if (!zip)
		{
			errMsg = "ExtractZipEntryToFile: zip handle is null";
			return false;
		}

		if (zip_entry_open(zip, entryName.c_str()) != 0)
		{
			errMsg = "ExtractZipEntryToFile: failed to open zip entry " + entryName;
			return false;
		}

		void* buf = nullptr;
		size_t size = 0;

		if (zip_entry_read(zip, &buf, &size) < 0)
		{
			zip_entry_close(zip);
			errMsg = "ExtractZipEntryToFile: failed to read zip entry " + entryName;
			return false;
		}

		zip_entry_close(zip);

		fs::path outPath = workingRoot / fs::path(entryName);
		fs::create_directories(outPath.parent_path());

		std::ofstream file(outPath, std::ios::binary);
		if (!file)
		{
			free(buf);
			errMsg = "ExtractZipEntryToFile: failed to open output file " + outPath.string();
			return false;
		}

		if (size > 0)
			file.write(static_cast<const char*>(buf), static_cast<std::streamsize>(size));

		free(buf);

		if (!file)
		{
			errMsg = "ExtractZipEntryToFile: failed to write output file " + outPath.string();
			return false;
		}

		return true;
	}

	std::optional<std::vector<std::uint8_t>> ReadBinaryEntry(zip_t* zip, const std::string& name)
	{
		if (!zip)
			return std::nullopt;

		if (zip_entry_open(zip, name.c_str()) != 0)
			return std::nullopt;

		void* buf = nullptr;
		size_t size = 0;

		if (zip_entry_read(zip, &buf, &size) < 0 || size == 0)
		{
			zip_entry_close(zip);
			return std::nullopt;
		}

		std::vector<std::uint8_t> bytes(
			static_cast<std::uint8_t*>(buf),
			static_cast<std::uint8_t*>(buf) + size
		);

		free(buf);
		zip_entry_close(zip);

		return bytes;
	}

	std::optional<ImageData> DecodeImageMemory(const std::vector<std::uint8_t>& bytes)
	{
		int width = 0;
		int height = 0;
		int sourceChannels = 0;

		// RGBA for consistency
		unsigned char* decoded = stbi_load_from_memory(
			bytes.data(),
			static_cast<int>(bytes.size()),
			&width,
			&height,
			&sourceChannels,
			4
		);

		if (!decoded)
			return std::nullopt;

		ImageData image;
		image.width = width;
		image.height = height;
		image.channels = 4;
		image.data.assign(decoded, decoded + width * height * 4);

		stbi_image_free(decoded);

		return image;
	}


	// ---------------- WRITE HELPERS ---------------- 
	static std::filesystem::path BuildWritePath(const std::string& writePath, const BCFIO::WriteConfig& cfg)
	{
		std::filesystem::path out(writePath);
		if (out.has_extension())
			return out; 	// check if write path has an extension and use it over cfg.extToUse
		if (!cfg.extToUse.empty())
			out.replace_extension(cfg.extToUse);
		return out;
	}

	static bool WriteXMLDocumentEntry(
		zip_t* zip, const std::string& entryName,
		const XMLLib::XMLDocumentHandle& doc, std::string& errMsg,
		bool prettyPrint = true)
	{
		if (!zip)
		{
			errMsg = "WriteXMLDocumentEntry: zip handle is null";
			return false;
		}

		if (!doc.IsValid())
		{
			errMsg = "WriteXMLDocumentEntry: invalid XML document for " + entryName;
			return false;
		}

		const std::string xml = doc.ToString(prettyPrint);
		if (xml.empty())
		{
			errMsg = "WriteXMLDocumentEntry: failed to serialize XML for " + entryName;
			return false;
		}

		if (zip_entry_open(zip, entryName.c_str()) != 0)
		{
			errMsg = "WriteXMLDocumentEntry: failed to open zip entry " + entryName;
			return false;
		}

		const int writeResult = zip_entry_write(zip, xml.data(), xml.size());
		if (writeResult != 0)
		{
			zip_entry_close(zip);
			errMsg = "WriteXMLDocumentEntry: failed to write zip entry " + entryName;
			return false;
		}

		if (zip_entry_close(zip) != 0)
		{
			errMsg = "WriteXMLDocumentEntry: failed to close zip entry " + entryName;
			return false;
		}

		return true;
	}

	static bool WriteDirectoryEntry(zip_t* zip, const std::string& dirName, std::string& errMsg)
	{
		if (!zip)
		{
			errMsg = "WriteDirectoryEntry: zip handle is null";
			return false;
		}

		std::string normalized = dirName;
		if (normalized.empty() || normalized.back() != '/')
			normalized += '/';

		if (zip_entry_open(zip, normalized.c_str()) != 0)
		{
			errMsg = "WriteDirectoryEntry: failed to open zip entry " + normalized;
			return false;
		}

		if (zip_entry_write(zip, "", 0) != 0)
		{
			zip_entry_close(zip);
			errMsg = "WriteDirectoryEntry: failed to write zip entry " + normalized;
			return false;
		}

		if (zip_entry_close(zip) != 0)
		{
			errMsg = "WriteDirectoryEntry: failed to close zip entry " + normalized;
			return false;
		}

		return true;
	}

	static bool WriteDocumentRefEntry(
		zip_t* zip, const std::string& entryName,
		const BCFDocumentRef& ref, std::string& errMsg,
		bool prettyPrint = true)
	{
		if (!ref.IsValid())
		{
			errMsg = "WriteDocumentRefEntry: invalid document ref for " + entryName;
			return false;
		}

		const XMLLib::XMLDocumentHandle* doc = ref.Get();
		if (!doc || !doc->IsValid())
		{
			errMsg = "WriteDocumentRefEntry: could not resolve document for " + entryName;
			return false;
		}

		return WriteXMLDocumentEntry(zip, entryName, *doc, errMsg, prettyPrint);
	}

	static bool WriteOptionalDocumentRefEntry(
		zip_t* zip, const std::string& entryName,
		const std::optional<BCFDocumentRef>& ref, std::string& errMsg,
		bool prettyPrint = true)
	{
		if (!ref.has_value())
			return true; // this is still valid
		return WriteDocumentRefEntry(zip, entryName, *ref, errMsg, prettyPrint);
	}

	static std::string ToZipEntryPath(const fs::path& relativePath)
	{
		return relativePath.generic_string();
	}

	static bool WriteBinaryFileEntry(
		zip_t* zip,
		const std::string& entryName,
		const fs::path& filePath,
		std::string& errMsg)
	{
		if (!zip)
		{
			errMsg = "WriteBinaryFileEntry: zip handle is null";
			return false;
		}

		std::ifstream file(filePath, std::ios::binary);
		if (!file)
		{
			errMsg = "WriteBinaryFileEntry: failed to open file " + filePath.string();
			return false;
		}

		std::vector<char> bytes((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());

		if (!file.good() && !file.eof())
		{
			errMsg = "WriteBinaryFileEntry: failed while reading file " + filePath.string();
			return false;
		}

		if (zip_entry_open(zip, entryName.c_str()) != 0)
		{
			errMsg = "WriteBinaryFileEntry: failed to open zip entry " + entryName;
			return false;
		}

		const int writeResult = zip_entry_write(zip, bytes.data(), bytes.size());
		if (writeResult != 0)
		{
			zip_entry_close(zip);
			errMsg = "WriteBinaryFileEntry: failed to write zip entry " + entryName;
			return false;
		}

		if (zip_entry_close(zip) != 0)
		{
			errMsg = "WriteBinaryFileEntry: failed to close zip entry " + entryName;
			return false;
		}

		return true;
	}

	static std::string ToZipPath(const fs::path& path)
	{
		return path.generic_string(); // always forward slashes
	}


	static void StbiWriteToVector(void* context, void* data, int size)
	{
		auto* out = static_cast<std::vector<std::uint8_t>*>(context);

		const auto* bytes = static_cast<const std::uint8_t*>(data);
		out->insert(out->end(), bytes, bytes + size);
	}

	static bool WriteImageEntry(
		zip_t* zip, const std::string& entryName, const BCFIO::WriteConfig& cfg,
		const ImageData& image, std::string& errMsg)
	{
		if (!zip)
		{
			errMsg = "[BCFIO Write] Write failed: invalid zip handle";
			return false;
		}

		if (image.width <= 0 || image.height <= 0 || image.channels <= 0 || image.data.empty())
		{
			errMsg = "[BCFIO Write] Write failed: invalid image data for: " + entryName;
			return false;
		}

		std::vector<std::uint8_t> encoded;
		auto lower = [](std::string s)
			{
				for (char& c : s)
					c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
				return s;
			};
		const std::string lowerName = lower(entryName);

		int ok = 0;

		auto saveAsJPEG = [&]() -> bool
		{
			const int quality = 90;
			return stbi_write_jpg_to_func(
				StbiWriteToVector,
				&encoded,
				image.width,
				image.height,
				image.channels,
				image.data.data(),
				quality
			) != 0;
		};

		auto saveAsPNG = [&]() -> bool
		{
			const int strideBytes = image.width * image.channels;
			return stbi_write_png_to_func(
				StbiWriteToVector,
				&encoded,
				image.width,
				image.height,
				image.channels,
				image.data.data(),
				strideBytes
			) != 0;
		};

		switch (cfg.snapshotSaveFormat)
		{
			case BCFIO::WriteConfig::PNG:
				ok = saveAsPNG();
				break;
			case BCFIO::WriteConfig::JPEG:
				ok = saveAsJPEG();
				break;
			case BCFIO::WriteConfig::ORIGINAL:
			{
				if (lowerName.ends_with(".jpg") || lowerName.ends_with(".jpeg"))
					 ok = saveAsJPEG();
				 else if (lowerName.ends_with(".png"))
					 ok = saveAsPNG();
				 else
				 {
					 errMsg = "[BCFIO Write] Write failed: unrecognized image extension for: " + entryName;
					 return false;
				 }
			}
		}

		if (!ok || encoded.empty())
		{
			errMsg = "[BCFIO Write] Write failed: could not encode image: " + entryName;
			return false;
		}

		if (zip_entry_open(zip, entryName.c_str()) != 0)
		{
			errMsg = "[BCFIO Write] Write failed: could not create zip entry: " + entryName;
			return false;
		}

		if (zip_entry_write(zip, encoded.data(), encoded.size()) < 0)
		{
			zip_entry_close(zip);
			errMsg = "[BCFIO Write] Write failed: could not write zip entry: " + entryName;
			return false;
		}

		zip_entry_close(zip);
		return true;
	}
}


BCFDocument BCFIO::Parse(const std::string& bcfFilePath, std::string& errMsg, const ParseConfig& cfg)
{
	BCFDocument result;
	// check if valid BCF file
	// > has extension .bcf or .bcfzip
	// > is a zip file
	const fs::path bcfPath{ bcfFilePath };
	if (!bcfPath.has_extension())
	{
		errMsg = "[BCFIO Parse] Invalid file path: No extension";
		return result;
	}
	if (bcfPath.extension() != ".bcf" && bcfPath.extension() != ".bcfzip")
	{
		errMsg = "[BCFIO Parse] Invalid file path: Wrong extension (must be .bcf or .bcfzip)";
		return result;
	}
	// will close the zip file if exited early
	ZipGuard zg(bcfFilePath.c_str(), 'r'); // r for read only
	if (!zg.valid())
	{
		errMsg = "[BCFIO Parse] Invalid file: Invalid zip file or not a zip file";
		return result;
	}
	int entryCount = zip_entries_total(zg.zip);
	if (entryCount <= 0)
	{
		errMsg = "[BCFIO Parse] Invalid file: Empty zip file";
		return result;
	}

	result.srcPath = bcfFilePath;

	// validate .version file
	// load into document store if there
	auto versDocOpt = ParseXMLEntry(zg.zip, versionFileName, errMsg);
	if (!versDocOpt)
		return result;
	result.version = BCFDocumentStore::Add(std::move(versDocOpt.value()));

	// load optional files into document store
	// if it cannot load up, BCF specifications say NOT to throw an error and simply
	// ignore these files. errMsg is cleared, so logging is required to see if any
	// errors actually occured here
	std::string optErrMsg = "";
	auto documentsDocOpt = ParseXMLEntry(zg.zip, documentsFileName, optErrMsg);
	if (documentsDocOpt.has_value())
	{
		result.documents = BCFDocumentStore::Add(std::move(documentsDocOpt.value()));
	}
	auto extensionsDocOpt = ParseXMLEntry(zg.zip, extensionsFileName, optErrMsg);
	if (extensionsDocOpt.has_value())
	{
		result.extensions = BCFDocumentStore::Add(std::move(extensionsDocOpt.value()));
	}
	auto projectDocOpt = ParseXMLEntry(zg.zip, projectFileName, optErrMsg);
	if (projectDocOpt.has_value())
	{
		result.project = BCFDocumentStore::Add(std::move(projectDocOpt.value()));
	}
	// Check for the additional documents folder
	// we just need to set a bool since it should always be a root level folder called Documents
	// and any access to those files just needs to access Documents/... 
	result.hasDocFolder = ZipHasFolder(zg.zip, documentsFolderName);
	/*
	for each entry in zip:
		if no '/' -> skip
		split at first '/'
		guid = before slash
		localName = after slash
		save guid as new topic, get it if alrdy existing
		validate .bcf markup file and load into document store (if cannot find markup, this topic is marked invalid)
		for each .bcfv file, load into document store
		for each .png/.jpg/.jpeg files, save name of snapshot
	*/
	for (int i = 0; i < entryCount; ++i)
	{
		if (zip_entry_openbyindex(zg.zip, i) != 0)
			continue;

		const char* rawName = zip_entry_name(zg.zip);
		std::string entryName = rawName ? rawName : "";

		zip_entry_close(zg.zip);

		if (entryName.empty())
			continue;

		// Skip root-level files like bcf.version, project.bcfp, etc.
		auto partsOpt = SplitTopicEntryPath(entryName);
		if (!partsOpt)
			continue;

		const auto& parts = *partsOpt;

		// validate GUID format (cheap check for now)
		if (parts.guid.size() != 36)
			continue;
		if (!(parts.guid[8] == '-' && parts.guid[13] == '-'
			&& parts.guid[18] == '-' && parts.guid[23] == '-'))
			continue;

		// add all valid topics to document, or get if it already exists
		auto& topic = result.topics[parts.guid];
		topic.guid = parts.guid;

		if (parts.localName == markupFileName)	// process markup.bcf, REQUIRED
		{
			auto markupDocOpt = ParseXMLEntry(zg.zip, entryName, errMsg);
			if (!markupDocOpt)
				continue; // markup is required for a topic once encountered, if it doesnt exist, stop parsing this entry

			topic.markup.doc = BCFDocumentStore::Add(std::move(*markupDocOpt));
		}
		else if (parts.localName.size() >= strlen(visualExt) &&	// process .bcfv (optional)
			parts.localName.ends_with(visualExt))
		{
			auto viewpointDocOpt = ParseXMLEntry(zg.zip, entryName, errMsg);
			if (viewpointDocOpt)
			{
				BCFViewpoint vp;
				vp.doc = BCFDocumentStore::Add(std::move(*viewpointDocOpt));
				vp.fileName = parts.localName;
				vp.guid = vp.doc.Get()->GetRootElement().GetAttribute("Guid");
				topic.viewpoints.push_back(std::move(vp));
			}
		}
		else if (IsSnapshotFile(parts.localName))
		{
			// parse snapshot into memory using stbi
			auto bytesOpt = ReadBinaryEntry(zg.zip, entryName); // read as binary
			if (!bytesOpt)
			{
				errMsg = "[BCFIO Parse] Warning: Could not read snapshot file: " + entryName;
			}

			auto imageOpt = DecodeImageMemory(*bytesOpt); // decode using stbi
			if (!imageOpt)
			{
				errMsg = "[BCFIO Parse] Warning: Could not decode snapshot image: " + entryName;
			}

			BCFSnapshot snapshot;
			snapshot.fileName = parts.localName;
			snapshot.image = std::move(*imageOpt);

			topic.snapshots.push_back(std::move(snapshot));
		}
	}

	result.valid = true;
	return result;
}

bool BCFIO::Write(const BCFDocument& bcfDoc, const std::string& writePath, std::string& errMsg, const WriteConfig& cfg)
{
	errMsg.clear();

	if (!bcfDoc.isValid())
	{
		errMsg = "[BCFIO Write] Write failed: BCF document is invalid";
		return false;
	}

	if (!bcfDoc.version.IsValid())
	{
		errMsg = "[BCFIO Write] Write failed: version document is missing";
		return false;
	}

	namespace fs = std::filesystem;
	const fs::path outPath = BuildWritePath(writePath, cfg);

	if (cfg.writeToNewFile && fs::exists(outPath))
	{
		errMsg = "[BCFIO Write] Write failed: output file already exists: " + outPath.string();
		return false;
	}
	else if (!cfg.writeToNewFile && fs::exists(outPath))
	{
		std::error_code ec;
		fs::remove(outPath, ec);
		if (ec)
		{
			errMsg = "[BCFIO Write] Write failed: could not remove existing output file: " + outPath.string();
			return false;
		}
	}

	ZipGuard zg(outPath.string().c_str(), 'w', ZIP_DEFAULT_COMPRESSION_LEVEL); // w for writing
	if (!zg.valid())
	{
		errMsg = "[BCFIO Write] Write failed: could not open output zip";
		return false;
	}

	// Root-level required doc
	if (!WriteDocumentRefEntry(zg.zip, versionFileName, bcfDoc.version, errMsg))
		return false; // could not write bcf.version

	// Root-level optional docs
	if (!WriteOptionalDocumentRefEntry(zg.zip, projectFileName, bcfDoc.project, errMsg))
		return false;

	if (!WriteOptionalDocumentRefEntry(zg.zip, documentsFileName, bcfDoc.documents, errMsg))
		return false;

	if (!WriteOptionalDocumentRefEntry(zg.zip, extensionsFileName, bcfDoc.extensions, errMsg))
		return false;

	// DEPRECATED:
	// Explicit Documents/ directory if working assets contain it or flag says it exists
	//fs::path const workingPath = bcfDoc.workingPath;
	//if (bcfDoc.hasDocumentsFolder)
	//{
	//	if (!WriteDirectoryEntry(zg.zip, documentsFolderName, errMsg))
	//		return false;
	//	// copy all the document files from the working path into the zip under Documents/.
	//	if (!workingPath.empty())
	//	{
	//		const fs::path docsRoot = workingPath / documentsFolderName;
	//		if (fs::exists(docsRoot) && fs::is_directory(docsRoot))
	//		{
	//			for (const auto& entry : fs::recursive_directory_iterator(docsRoot))
	//			{
	//				const fs::path fullPath = entry.path();
	//				const fs::path relPath = fs::relative(fullPath, workingPath);
	//				const std::string zipPath = ToZipEntryPath(relPath);
	//				if (entry.is_directory())
	//				{
	//					if (!WriteDirectoryEntry(zg.zip, zipPath, errMsg))
	//						continue;
	//				}
	//				else if (entry.is_regular_file())
	//				{
	//					if (!WriteBinaryFileEntry(zg.zip, zipPath, fullPath, errMsg))
	//						continue;
	//				}
	//			}
	//		}
	//	}
	//}

	for (const auto& [guid, topic] : bcfDoc.topics)
	{
		if (!topic.isValid())
			continue;

		// Explicit topic folder entry writing
		// BCF XML specifications state to be a valid .bcfzip directory entries MUST exist in the file
		// that is if there is a guidxxx1/markup.bcf, there MUST BE ONE & only ONE guixxx1/ entry
		// basically one entry per topic folder, plus it's contents
		if (!WriteDirectoryEntry(zg.zip, guid, errMsg))
			return false;

		// Required markup
		if (!WriteDocumentRefEntry(zg.zip, guid + "/" + markupFileName, topic.markup.doc, errMsg))
			return false;

		int unnamedVpIndex = 0;

		// write all viewpoints
		for (const auto& vp : topic.viewpoints)
		{
			auto& vpDocRef = vp.doc;
			if (!vpDocRef.IsValid())
				continue;

			const auto* vpDoc = vpDocRef.Get();
			if (!vpDoc || !vpDoc->IsValid())
				continue;

			std::string vpGuid = vpDoc->GetRootElement().GetAttribute("Guid");
			std::string entryName;

			if (!vpGuid.empty())
			{
				entryName = guid + "/" + vpGuid + (cfg.viewpointsHaveTailName ? "_viewpoint" : "") + visualExt;
			}
			else
			{
				// default naming following:
				//	.bcfv files are saved as their uuid (<VisualizationInfo Guid="356746a8-2f2a-4757-b13b-911b7b218c2a">),
				//	if no uuid, save as viewpoint.bcfv. If multiple with no uuid [should NOT be the case], label as viewpointN.bcfv,
				//  where N is 1 to N. (the first viewpoint with no uuid is always viewpoint.bcfv)
				entryName = guid + "/viewpoint";
				if (unnamedVpIndex > 0)
					entryName += std::to_string(unnamedVpIndex);
				entryName += visualExt;
				++unnamedVpIndex;
			}

			if (!WriteDocumentRefEntry(zg.zip, entryName, vpDocRef, errMsg))
				return false;
		}

		// write all snapshots
		for (const auto& snap : topic.snapshots)
		{
			if (snap.image.data.empty())
				continue;

			std::string snapName = snap.fileName;
			const std::string entryName = guid + "/" + snapName;

			if (!WriteImageEntry(zg.zip, entryName, cfg, snap.image, errMsg))
				return false;
		}
	}

	return true;
}