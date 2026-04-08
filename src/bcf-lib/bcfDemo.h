#pragma once
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
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
	bool ImportTestXML();
	bool ImportTestXSD();
	void DisplayXMLImported();

private:
	std::string m_xmlInputPath = "";
	std::string m_xsdInputPath = "";
	bool useHeaderXSD = false;

	std::string m_xmlRawText = "";

	std::string m_loadedXSDText;
	std::string m_status = "Idle";
	std::string m_lastError;
};

inline void BCFDemo::OnPreSelectable()
{
	ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(150, 69, 255, 255));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(169, 89, 255, 255));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(186, 29, 255, 255));
}

inline void BCFDemo::DrawSelectedDemo()
{
	ImGui::InputText("XSD File Path", &m_xsdInputPath);

	if (ImGui::Button("Import XSD"))
	{
		ImportTestXSD();
	}

	ImGui::Checkbox("Use Header XSD", &useHeaderXSD);

	ImGui::Separator();

	ImGui::InputText("XML File Path", &m_xmlInputPath);

	if (ImGui::Button("Import XML"))
	{
		ImportTestXML();
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
	ImGui::Text("Status: %s", m_status.c_str());

	if (!m_lastError.empty())
	{
		ImGui::Separator();
		ImGui::TextWrapped("Last Error: %s", m_lastError.c_str());
	}

	ImGui::Separator();
	ImGui::TextWrapped("Loaded XSD chars: %d", static_cast<int>(m_loadedXSDText.size()));
	ImGui::TextWrapped("Using header XSD: %s", useHeaderXSD ? "true" : "false");

	DisplayXMLImported();
}