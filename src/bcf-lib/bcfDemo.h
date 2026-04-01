#pragma once
#include "imgui.h"
#include "demo_module.h"
#include <string>
#include <vector>

class BCFDemo : public DemoModule
{
public:
	BCFDemo() : DemoModule("BCF", "BCF Demo Panel") {}

protected:
	void OnPreSelectable() override;
	void DrawSelectedDemo() override;
	void OnPostSelectable() override;

	void OnPrePanel() override;
	void DrawDemoPanel() override;

private:
	bool ImportTestXml();
	bool ExportAndZipTestXml();

private:
	std::string m_Status = "Idle";
	std::string m_LastError;
	std::string m_InputPath = "C:/Users/Admin/Desktop/TESTING DATA FOLDER/test_input.xml";
	std::string m_OutputXmlPath = "C:/Users/Admin/Desktop/TESTING DATA FOLDER/exported_test.xml";
	std::string m_OutputZipPath = "C:/Users/Admin/Desktop/TESTING DATA FOLDER/exported_test.zip";

	std::string m_RootName;
	std::string m_Title;
	std::string m_Message;
	std::vector<std::string> m_Items;
};

inline void BCFDemo::OnPreSelectable()
{
	ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(150, 69, 255, 255));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(169, 89, 255, 255));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(186, 29, 255, 255));
}

inline void BCFDemo::DrawSelectedDemo()
{
	if (ImGui::Button("Import XML"))
	{
		if (ImportTestXml())
			m_Status = "Import succeeded";
		else
			m_Status = "Import failed";
	}

	ImGui::SameLine();

	if (ImGui::Button("Export & ZIP XML"))
	{
		if (ExportAndZipTestXml())
			m_Status = "Export + ZIP succeeded";
		else
			m_Status = "Export + ZIP failed";
	}
}

inline void BCFDemo::OnPostSelectable()
{
	ImGui::PopStyleColor(3);
}

inline void BCFDemo::OnPrePanel()
{
	ImGui::SetNextWindowSize(ImVec2(500, 620), ImGuiCond_Appearing);
}

inline void BCFDemo::DrawDemoPanel()
{
	ImGui::Text("Status: %s", m_Status.c_str());

	if (!m_LastError.empty())
	{
		ImGui::TextWrapped("Last error: %s", m_LastError.c_str());
	}

	ImGui::Separator();
	ImGui::Text("Input XML: %s", m_InputPath.c_str());
	ImGui::Text("Output XML: %s", m_OutputXmlPath.c_str());
	ImGui::Text("Output ZIP: %s", m_OutputZipPath.c_str());

	ImGui::Separator();
	ImGui::Text("Parsed XML");

	ImGui::Text("Root: %s", m_RootName.c_str());
	ImGui::Text("Title: %s", m_Title.c_str());
	ImGui::TextWrapped("Message: %s", m_Message.c_str());

	ImGui::Text("Items:");
	for (const std::string& item : m_Items)
	{
		ImGui::BulletText("%s", item.c_str());
	}
}