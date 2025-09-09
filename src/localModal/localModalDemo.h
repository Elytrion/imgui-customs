#pragma once
#include "imguiLMP.h"
#include "demo_module.h"

class LocalModalDemo : public DemoModule
{
public:
    LocalModalDemo() : DemoModule("Local Modal Popups", "Local Modal Popup Demo Panel") {}
protected:
	void DrawSelectedDemo();
    void OnPrePanel() override;
	void DrawDemoPanel() override;
};

inline void LocalModalDemo::DrawSelectedDemo()
{
    ImGui::NewLine();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    if (ImGui::Button("Open Local Popup"))
    {
        ImGui::OpenLocalPopupModal("Main Local Panel Popup");
    }
	ImGui::PopStyleColor();
    ImGui::NewLine();
    ImGui::SetNextWindowSize({ 100.0f, 0.0f }, ImGuiCond_Appearing);
    if (ImGui::BeginLocalPopupModal("Main Local Panel Popup", 0.2f))
    {
        ImGui::Text("This is a local popup modal."); 
        ImGui::Text("It will prevent access to the window it is on, while leaving other windows still accessible");
        ImGui::Text("Close this popup to gain access to the window.");
        ImGui::Spacing();
        if (ImGui::SmallButton("Close"))
        {
            ImGui::CloseLocalPopupModal("Main Local Panel Popup");
        }
        ImGui::EndLocalPopupModal();
    }
}

inline void LocalModalDemo::OnPrePanel()
{
    ImGui::SetNextWindowSize({ 500.0f, 300.0f }, ImGuiCond_Appearing);
}

inline void LocalModalDemo::DrawDemoPanel()
{
    static int   anchor_idx = 0;
    static bool  clamp_inside = false; // no longer needed, but left here if you want to keep a toggle
    static ImVec2 offset_px = ImVec2(0, 0);
    static ImVec2 custom_anchor_norm = ImVec2(0.5f, 0.5f);
    static float dim_bg_alpha = 0.35f;

    static const char* kAnchorLabels[] = {
        "Center",
        "Top-Left", "Top", "Top-Right",
        "Left",               "Right",
        "Bottom-Left", "Bottom", "Bottom-Right",
        "Custom (normalized)"
    };

    ImGui::SeparatorText("Local Popup Controls (this window is the host)");
    ImGui::TextWrapped("Open a popup constrained to THIS window. Other windows remain fully interactive.");
    ImGui::Spacing();

    ImGui::SetNextItemWidth(220.0f);
    ImGui::Combo("Anchor", &anchor_idx, kAnchorLabels, IM_ARRAYSIZE(kAnchorLabels));

    if (anchor_idx == 9)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("(x,y in [0..1])");
        ImGui::SliderFloat2("Custom Anchor (norm)", &custom_anchor_norm.x, 0.0f, 1.0f, "%.2f");
    }

    ImGui::SliderFloat2("Pixel Offset", &offset_px.x, -200.0f, 200.0f, "%.0f");
    ImGui::SliderFloat("Dim BG Alpha", &dim_bg_alpha, 0.0f, 1.0f, "%.2f");

    ImGui::Spacing();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    if (ImGui::Button("Open Local Popup"))
        ImGui::OpenLocalPopupModal("LocalDemoPopup");
    ImGui::PopStyleColor();

    // --- Compute desired anchor & pivot BEFORE BeginLocalPopupModal ---
    // Use the current window as host (this call-site *is* the host)
    ImGuiWindow* host = ImGui::GetCurrentWindow();
    const ImVec2 host_min = host->Pos;
    const ImVec2 host_max = ImVec2(host->Pos.x + host->Size.x, host->Pos.y + host->Size.y);

    auto corner = [&](float nx, float ny) {
        return ImVec2(ImLerp(host_min.x, host_max.x, nx),
            ImLerp(host_min.y, host_max.y, ny));
        };

    ImVec2 pivot(0.5f, 0.5f);
    ImVec2 anchor = corner(0.5f, 0.5f); // default center
    switch (anchor_idx)
    {
    case 0: anchor = corner(0.5f, 0.5f); pivot = ImVec2(0.5f, 0.5f); break; // Center
    case 1: anchor = corner(0.0f, 0.0f); pivot = ImVec2(0.0f, 0.0f); break; // TL
    case 2: anchor = corner(0.5f, 0.0f); pivot = ImVec2(0.5f, 0.0f); break; // T
    case 3: anchor = corner(1.0f, 0.0f); pivot = ImVec2(1.0f, 0.0f); break; // TR
    case 4: anchor = corner(0.0f, 0.5f); pivot = ImVec2(0.0f, 0.5f); break; // L
    case 5: anchor = corner(1.0f, 0.5f); pivot = ImVec2(1.0f, 0.5f); break; // R
    case 6: anchor = corner(0.0f, 1.0f); pivot = ImVec2(0.0f, 1.0f); break; // BL
    case 7: anchor = corner(0.5f, 1.0f); pivot = ImVec2(0.5f, 1.0f); break; // B
    case 8: anchor = corner(1.0f, 1.0f); pivot = ImVec2(1.0f, 1.0f); break; // BR
    case 9: anchor = corner(custom_anchor_norm.x, custom_anchor_norm.y);
        pivot = ImVec2(0.5f, 0.5f); break;                               // Custom
    }

    // --- Draw the local popup; its position is now defined by SetNextWindowPos+Pivot ---
    if (ImGui::BeginLocalPopupModal("LocalDemoPopup", nullptr, 0, dim_bg_alpha, pivot))
    {
        // ESC = safety close
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            ImGui::CloseLocalPopupModal("LocalDemoPopup");

        // Header (title + X on the right)
        {
            const float header_h = ImGui::GetFrameHeight();
            ImGui::BeginChild("##lpm_header", ImVec2(0, header_h), false,
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            ImGui::TextUnformatted("Local Popup");

            float close_w = header_h;
            float x = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - close_w;
            ImGui::SameLine();
            ImGui::SetCursorPosX(x);
            if (ImGui::SmallButton("X"))
                ImGui::CloseLocalPopupModal("LocalDemoPopup");

            ImGui::EndChild();
        }
        ImGui::Separator();

        // Body
        ImGui::Text("Anchor: %s", kAnchorLabels[anchor_idx]);
        if (anchor_idx == 9)
            ImGui::Text("Custom (%.2f, %.2f) + Offset (%.0f, %.0f)",
                custom_anchor_norm.x, custom_anchor_norm.y, offset_px.x, offset_px.y);
        else
            ImGui::Text("Offset (%.0f, %.0f)", offset_px.x, offset_px.y);

        ImGui::Spacing();
        ImGui::TextWrapped("Note: This popup only blocks this window; other windows stay responsive.");

        ImGui::Spacing();
        if (ImGui::SmallButton("Close"))
            ImGui::CloseLocalPopupModal("LocalDemoPopup");

        ImGui::EndLocalPopupModal();
    }
}
