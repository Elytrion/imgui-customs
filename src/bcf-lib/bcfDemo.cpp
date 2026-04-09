#include "bcfDemo.h"
#include "xmllib/XMLParser.h"
#include "xmllib/XMLSchema.hpp"
#include "xmllib/XMLMain.hpp"
#include "TestSchemas.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

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
	XMLDocumentHandle doc;
	if (schema.isValid())
		doc = XMLParser::Parse(m_xmlInputPath, schema);
	else
		doc = XMLParser::Parse(m_xmlInputPath);

	if (!doc.IsValid())
	{
		m_status = "Import XML failed";
		m_lastError = doc.GetLastError();
		return false;
	}

	m_status = "Import XML succeeded";

	m_parsedDocument = std::move(doc);

	m_xmlRawText = m_parsedDocument.ToString();

	return true;
}

void BCFDemo::DisplayXMLImported()
{
	if (!m_parsedDocument.IsValid())
		return;

	ImGui::Separator();
	ImGui::Text("Imported Raw XML Text");

	ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly;
	ImGui::InputTextMultiline(
		"##ImportedXML",
		&m_xmlRawText,
		ImVec2(-FLT_MIN, 100.0f),
		flags
	);

	ImGui::Separator();
	ImGui::Text("XML Tree (From Memory Representation)");

	auto root = m_parsedDocument.GetRootElement(); // or GetDocumentElement()
	if (root.IsValid())
	{
		DrawXMLNodeTree(root);
	}
}

static bool IsWhitespaceOnly(const std::string& s)
{
	for (char c : s)
	{
		if (!std::isspace(static_cast<unsigned char>(c)))
			return false;
	}
	return true;
}

void BCFDemo::DrawXMLNodeTree(const XMLLib::XMLNodeHandle& node)
{
	if (!node.IsValid())
		return;

	ImGui::PushID(node.GetRawNode());

	using namespace XMLLib;

	const XMLNodeType type = node.GetType();

	if (type == XMLNodeType::Text)
	{
		const std::string text = node.GetValue().empty() ? node.GetText() : node.GetValue();
		if (!IsWhitespaceOnly(text))
			ImGui::BulletText("#text: %s", text.c_str());

		ImGui::PopID();
		return;
	}

	if (type == XMLNodeType::Comment)
	{
		ImGui::BulletText("<!-- %s -->", node.GetValue().c_str());
		ImGui::PopID();
		return;
	}

	if (type == XMLNodeType::CData)
	{
		ImGui::BulletText("<![CDATA[%s]]>", node.GetValue().c_str());
		ImGui::PopID();
		return;
	}

	const std::vector<XMLAttribute> attributes = node.GetAttributes();
	const std::vector<XMLNodeHandle> children = node.GetChildren();

	std::vector<XMLNodeHandle> visibleChildren;
	visibleChildren.reserve(children.size());

	for (const auto& child : children)
	{
		if (!child.IsValid())
			continue;

		if (child.GetType() == XMLNodeType::Text)
		{
			const std::string text = child.GetValue().empty() ? child.GetText() : child.GetValue();
			if (IsWhitespaceOnly(text))
				continue;
		}

		visibleChildren.push_back(child);
	}

	if (node.IsElement() &&
		attributes.empty() &&
		visibleChildren.size() == 1 &&
		visibleChildren[0].GetType() == XMLNodeType::Text)
	{
		const std::string text = visibleChildren[0].GetValue().empty()
			? visibleChildren[0].GetText()
			: visibleChildren[0].GetValue();

		ImGui::BulletText("%s: %s", node.GetName().c_str(), text.c_str());
		ImGui::PopID();
		return;
	}

	std::string label = node.GetName();
	if (label.empty())
		label = "<unnamed>";

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
	if (visibleChildren.empty() && attributes.empty())
		flags |= ImGuiTreeNodeFlags_Leaf;

	const bool open = ImGui::TreeNodeEx(label.c_str(), flags);
	if (open)
	{
		for (const auto& attr : attributes)
			ImGui::BulletText("@%s = \"%s\"", attr.name.c_str(), attr.value.c_str());

		for (const auto& child : visibleChildren)
			DrawXMLNodeTree(child);

		ImGui::TreePop();
	}

	ImGui::PopID();
}