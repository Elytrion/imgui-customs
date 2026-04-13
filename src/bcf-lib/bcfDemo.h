#pragma once
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "demo_module.h"
#include <string>
#include "BCFIO.h"

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
    bool ImportTestBCF();
    void DisplayBCFImported();
    void DrawXMLNodeTree(const XMLLib::XMLNodeHandle& root);
    void DrawDocumentRefTree(const char* label, const DocumentRef& ref);
    void DrawOptionalDocumentRefTree(const char* label, const std::optional<DocumentRef>& ref);

private:
    std::string m_bcfInputPath;
    std::string m_status = "Idle";
    std::string m_lastError;

    BCFDocument m_loadedBCF;
};

inline void BCFDemo::OnPreSelectable()
{
    ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(150, 69, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(169, 89, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(186, 29, 255, 255));
}

inline void BCFDemo::DrawSelectedDemo()
{
    ImGui::InputText("BCF File Path", &m_bcfInputPath);

    if (ImGui::Button("Import BCF"))
    {
        ImportTestBCF();
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

    DisplayBCFImported();
}