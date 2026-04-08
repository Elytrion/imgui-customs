#include "bcfDemo.h"
#include "xmllib/XMLParser.h"
#include "xmllib/XMLSchema.hpp"
#include "xmllib/XMLMain.hpp"
#include "TestSchemas.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;
using namespace XMLLib;

bool BCFDemo::ImportTestXSD()
{
	m_lastError.clear();

	if (useHeaderXSD)
	{
		m_loadedXSDText = TEST_NOTE_XSD;
		m_status = "Loaded XSD from header";
		return true;
	}

	if (m_xsdInputPath.empty())
	{
		m_status = "Import XSD failed";
		m_lastError = "XSD path is empty";
		return false;
	}

	std::ifstream file(m_xsdInputPath, std::ios::binary);
	if (!file)
	{
		m_status = "Import XSD failed";
		m_lastError = "Failed to open XSD file";
		return false;
	}

	std::ostringstream oss;
	oss << file.rdbuf();
	m_loadedXSDText = oss.str();

	m_status = "Loaded XSD from file";
	return true;
}

bool BCFDemo::ImportTestXML()
{
	m_lastError.clear();

	if (m_xmlInputPath.empty())
	{
		m_status = "Import XML failed";
		m_lastError = "XML path is empty";
		return false;
	}

	if (!IsXMLLibInit())
	{
		InitXMLLib();
	}

	XMLSchema schema("test_note.xsd");

	if (useHeaderXSD && !m_loadedXSDText.empty())
	{
		schema.rawText = useHeaderXSD ? TEST_NOTE_XSD : m_loadedXSDText;
	}
	else if (!m_xsdInputPath.empty())
	{
		schema.schemaFilePath = m_xsdInputPath;
	}

	XMLDocumentHandle doc = XMLParser::Parse(m_xmlInputPath, schema);

	if (!doc.IsValid())
	{
		m_status = "Import XML failed";
		m_lastError = doc.GetLastError();
		return false;
	}

	m_status = "Import XML succeeded";

	m_xmlRawText = doc.toString();

	return true;
}

void BCFDemo::DisplayXMLImported()
{
	if (m_xmlRawText.empty())
		return;

	ImGui::Separator();
	ImGui::Text("Imported XML");

	ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly;
	ImGui::InputTextMultiline(
		"##ImportedXML",
		&m_xmlRawText,
		ImVec2(-FLT_MIN, 300.0f),
		flags
	);
}