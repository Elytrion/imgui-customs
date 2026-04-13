#include "bcfDemo.h"
#include <filesystem>
#include <iostream>


namespace fs = std::filesystem;
using namespace XMLLib;

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

bool BCFDemo::ImportTestBCF()
{
    m_lastError.clear();
    m_loadedBCF = BCFDocument{};

    if (m_bcfInputPath.empty())
    {
        m_status = "Import BCF failed";
        m_lastError = "BCF path is empty";
        return false;
    }

    m_loadedBCF = BCFIO::Parse(m_bcfInputPath, m_lastError);

    if (!m_loadedBCF.valid)
    {
        m_status = "Import BCF failed";
        if (m_lastError.empty())
            m_lastError = "BCF parse failed";
        return false;
    }

    m_status = "Import BCF succeeded";
    return true;
}

void BCFDemo::DrawDocumentRefTree(const char* label, const DocumentRef& ref)
{
    if (!ref.IsValid())
        return;

    const XMLLib::XMLDocumentHandle* doc = ref.Get();
    if (!doc || !doc->IsValid())
        return;

    auto root = doc->GetRootElement();
    if (!root.IsValid())
        return;

    if (ImGui::TreeNode(label))
    {
        DrawXMLNodeTree(root);
        ImGui::TreePop();
    }
}

void BCFDemo::DrawOptionalDocumentRefTree(const char* label, const std::optional<DocumentRef>& ref)
{
    if (!ref.has_value())
        return;

    DrawDocumentRefTree(label, *ref);
}

void BCFDemo::DisplayBCFImported()
{
    if (!m_loadedBCF.valid)
        return;

    ImGui::Separator();
    ImGui::Text("BCF Summary");
    ImGui::BulletText("Has Documents folder: %s", m_loadedBCF.hasDocumentsFolder ? "true" : "false");
    ImGui::BulletText("Topic count: %d", static_cast<int>(m_loadedBCF.topics.size()));

    ImGui::Separator();
    ImGui::Text("Root Documents");

    DrawDocumentRefTree("bcf.version", m_loadedBCF.versionDoc);
    DrawOptionalDocumentRefTree("project.bcfp", m_loadedBCF.projectDoc);
    DrawOptionalDocumentRefTree("documents.xml", m_loadedBCF.documentsDoc);
    DrawOptionalDocumentRefTree("extensions.xml", m_loadedBCF.extensionsDoc);

    ImGui::Separator();
    ImGui::Text("Topics");

    for (auto& [guid, topic] : m_loadedBCF.topics)
    {
        ImGui::PushID(guid.c_str());

        std::string topicLabel = guid;
        if (!topic.valid)
            topicLabel += " (invalid)";

        if (ImGui::TreeNode(topicLabel.c_str()))
        {
            ImGui::Text("GUID: %s", topic.guid.c_str());
            ImGui::Text("Valid: %s", topic.valid ? "true" : "false");

            DrawDocumentRefTree("markup.bcf", topic.markupDoc);

            if (!topic.viewpointDoc.empty())
            {
                if (ImGui::TreeNode("Viewpoints"))
                {
                    for (size_t i = 0; i < topic.viewpointDoc.size(); ++i)
                    {
                        std::string label = "viewpoint " + std::to_string(i);
                        DrawDocumentRefTree(label.c_str(), topic.viewpointDoc[i]);
                    }
                    ImGui::TreePop();
                }
            }

            if (!topic.snapshotNames.empty())
            {
                if (ImGui::TreeNode("Snapshots"))
                {
                    for (const auto& name : topic.snapshotNames)
                    {
                        ImGui::BulletText("%s", name.c_str());
                    }
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}