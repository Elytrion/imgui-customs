#include "bcfDemo.h"

#include <pugixml.hpp>
#include <zip.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

static std::string ZipErrorToString(int errorCode)
{
	char buffer[256] = {};
	zip_error_to_str(buffer, sizeof(buffer), errorCode, errno);
	return std::string(buffer);
}

bool BCFDemo::ImportTestXml()
{
	m_LastError.clear();
	m_RootName.clear();
	m_Title.clear();
	m_Message.clear();
	m_Items.clear();

	pugi::xml_document doc;
	const pugi::xml_parse_result result = doc.load_file(m_InputPath.c_str());

	if (!result)
	{
		m_LastError = std::string("Failed to load XML: ") + result.description();
		return false;
	}

	const pugi::xml_node root = doc.child("demo");
	if (!root)
	{
		m_LastError = "Missing root node <demo>";
		return false;
	}

	m_RootName = root.name();
	m_Title = root.child("title").text().as_string();
	m_Message = root.child("message").text().as_string();

	const pugi::xml_node itemsNode = root.child("items");
	for (pugi::xml_node item = itemsNode.child("item"); item; item = item.next_sibling("item"))
	{
		m_Items.push_back(item.text().as_string());
	}

	return true;
}

bool BCFDemo::ExportAndZipTestXml()
{
	m_LastError.clear();

	try
	{
		fs::create_directories(fs::path(m_OutputXmlPath).parent_path());
		fs::create_directories(fs::path(m_OutputZipPath).parent_path());
	}
	catch (const std::exception& e)
	{
		m_LastError = std::string("Failed to create output directories: ") + e.what();
		return false;
	}

	// Build an XML document from current in-memory state.
	pugi::xml_document doc;

	pugi::xml_node root = doc.append_child("demo");
	root.append_child("title").text().set(
		m_Title.empty() ? "Fallback Export Title" : m_Title.c_str());
	root.append_child("message").text().set(
		m_Message.empty() ? "Fallback Export Message" : m_Message.c_str());

	pugi::xml_node itemsNode = root.append_child("items");

	if (m_Items.empty())
	{
		itemsNode.append_child("item").text().set("Example A");
		itemsNode.append_child("item").text().set("Example B");
		itemsNode.append_child("item").text().set("Example C");
	}
	else
	{
		for (const std::string& itemText : m_Items)
		{
			itemsNode.append_child("item").text().set(itemText.c_str());
		}
	}

	if (!doc.save_file(m_OutputXmlPath.c_str(), PUGIXML_TEXT("  ")))
	{
		m_LastError = "Failed to save output XML";
		return false;
	}

	// Read XML file back into memory for zipping.
	std::ifstream inFile(m_OutputXmlPath, std::ios::binary);
	if (!inFile)
	{
		m_LastError = "Failed to reopen output XML for zipping";
		return false;
	}

	std::ostringstream oss;
	oss << inFile.rdbuf();
	const std::string xmlContent = oss.str();

	int zipError = 0;
	zip_t* archive = zip_open(m_OutputZipPath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &zipError);
	if (!archive)
	{
		m_LastError = std::string("Failed to create zip archive: ") + ZipErrorToString(zipError);
		return false;
	}

	zip_source_t* source = zip_source_buffer(
		archive,
		xmlContent.data(),
		xmlContent.size(),
		0 // libzip will not free this buffer
	);

	if (!source)
	{
		m_LastError = "Failed to create zip source";
		zip_discard(archive);
		return false;
	}

	// Put the XML inside the zip under a friendly internal path.
	if (zip_file_add(archive, "exported_test.xml", source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0)
	{
		m_LastError = std::string("Failed to add file to zip: ") + zip_strerror(archive);
		zip_source_free(source);
		zip_discard(archive);
		return false;
	}

	if (zip_close(archive) < 0)
	{
		m_LastError = "Failed to finalize zip archive";
		zip_discard(archive);
		return false;
	}

	return true;
}