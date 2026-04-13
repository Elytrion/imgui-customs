#include "BCFIO.h"
#include "xmllib/XMLParser.h"
#include <filesystem>
#include <iostream>
#include "zip.h"

namespace fs = std::filesystem;
namespace
{
	// taken from https://github.com/buildingSMART/BCF-XML/tree/release_3_0/Documentation
	constexpr const char* versionFileName		= "bcf.version";
	constexpr const char* documentsFileName		= "documents.xml";
	constexpr const char* documentsFolderName   = "Documents";
	constexpr const char* extensionsFileName	= "extensions.xml";
	constexpr const char* projectFileName		= "project.bcfp";
	constexpr const char* markupFileName		= "markup.bcf";
	constexpr const char* visualExt				= ".bcfv";
	constexpr const char* validSnapshotExt[3]	= { ".png",".jpg",".jpeg"};

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

		explicit ZipGuard(const std::string& path)
		{
			zip = zip_open(path.c_str(), 0, 'r');
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
			errOut = "Invalid format: Could not find " + name;
			return std::nullopt;
		}
		auto versOpt = ReadXMLEntry(zip, name); // attempt to read text from file
		if (!versOpt)
		{
			errOut = "Invalid files: Could not open and read " + name;
			return std::nullopt;
		}
		auto versDocHandle = XMLLib::XMLParser::ParseMemory(versionFileName, versOpt.value()); // Attempt to parse xml into DOMdocument format
		if (!versDocHandle.IsValid())
		{
			errOut = "Invalid Files: Could not DOM parse xml from " + name;
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
		errMsg = "Invalid file path: No extension";
		return result;
	}
	if (bcfPath.extension() != ".bcf" && bcfPath.extension() != ".bcfzip")
	{
		errMsg = "Invalid file path: Wrong extension (must be .bcf or .bcfzip)";
		return result;
	}

	ZipGuard zg(bcfFilePath.c_str()); // auto close the zip file if returned early
	if (!zg.valid())
	{
		errMsg = "Invalid file: Invalid zip file or not a zip file";
		return result;
	}
	int entryCount = zip_entries_total(zg.zip);
	if (entryCount <= 0)
	{
		errMsg = "Invalid file: Empty zip file";
		return result;
	}

	// validate .version file
	// load into document store if there
	auto versDocOpt = ParseXMLEntry(zg.zip, versionFileName, errMsg);
	if (!versDocOpt)
		return result;
	result.versionDoc = DocumentStore::Add(std::move(versDocOpt.value()));

	// load optional files into document store
	// if it cannot load up, BCF specifications say NOT to throw an error and simply
	// ignore these files. errMsg is cleared, so logging is required to see if any
	// errors actually occured here
	auto documentsDocOpt = ParseXMLEntry(zg.zip, documentsFileName, errMsg);
	if (documentsDocOpt.has_value())
	{
		result.documentsDoc = DocumentStore::Add(std::move(documentsDocOpt.value()));
	}
	auto extensionsDocOpt = ParseXMLEntry(zg.zip, extensionsFileName, errMsg);
	if (extensionsDocOpt.has_value())
	{
		result.extensionsDoc = DocumentStore::Add(std::move(extensionsDocOpt.value()));
	}
	auto projectDocOpt = ParseXMLEntry(zg.zip, projectFileName, errMsg);
	if (projectDocOpt.has_value())
	{
		result.projectDoc = DocumentStore::Add(std::move(projectDocOpt.value()));
	}
	// Check for the additional documents folder
	// we just need to set a bool since it should always be a root level folder called Documents
	// and any access to those files just needs to access Documents/... 
	result.hasDocumentsFolder = ZipHasFolder(zg.zip, documentsFolderName);
	errMsg.clear();

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
			topic.valid = true;
			topic.markupDoc = DocumentStore::Add(std::move(*markupDocOpt));
		}
		else if (parts.localName.size() >= strlen(visualExt) &&	// process .bcfv (optional)
			parts.localName.ends_with(visualExt))
		{
			auto viewpointDocOpt = ParseXMLEntry(zg.zip, entryName, errMsg);
			if (viewpointDocOpt)
				topic.viewpointDoc.push_back(DocumentStore::Add(std::move(*viewpointDocOpt)));
		}
		else //snapshot name saving
		{
			for (const char* ext : validSnapshotExt) // iterate through all valid snapshot exts
			{
				std::string extStr = ext;
				if (parts.localName.size() >= extStr.size() &&
					parts.localName.ends_with(extStr))
				{
					topic.snapshotNames.push_back(parts.localName);
					break;
				}
			}
		}
	}
}