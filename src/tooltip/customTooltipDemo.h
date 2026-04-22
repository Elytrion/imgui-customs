#pragma once
#include "imguiCustomTooltip.h"
#include "demo_module.h"

class CustomTooltipDemo : public DemoModule
{
public:
	CustomTooltipDemo() : DemoModule("Custom Tooltip", "Custom Tooltip Demo Panel"){}
protected:
	void DrawSelectedDemo();
	void OnPrePanel() override;
	void DrawDemoPanel() override;
};

inline void CustomTooltipDemo::DrawSelectedDemo()
{
	ImGui::Spacing();
	ImGui::Text("Hover over me to see a custom tooltip.");
	ImGui::DrawCustomTooltip("This is a custom tooltip.");
	ImGui::Spacing();
	ImGui::Text("Hover (Top pivot tooltip)");
	ImGui::DrawCustomTooltip(
		"This tooltip is anchored to the top of the item.",
		ImVec2{ 0.5f, 0.0f }, // pivot
		ImVec2{ 0.0f, -5.0f }   // offset
	);
	ImGui::Spacing();
	ImGui::Text("Hover (Bottom Right pivot tooltip)");
	ImGui::DrawCustomTooltip(
		"This tooltip is anchored to the bottom-right of the item.",
		ImVec2{ 1.0f, 1.0f }, // pivot
		ImVec2{ 5.0f, 5.0f }   // offset
	);
	ImGui::Separator();
	ImGui::TextWrapped("Tooltips will by default attempt to stay within the viewport, so the tooltip may not appear exactly at the specified pivot/offset if it would go outside the viewport. Try resizing the window to see this behavior. This behavior can be disabled by the user.");
}

inline void CustomTooltipDemo::OnPrePanel()
{
	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Appearing);
}

inline void CustomTooltipDemo::DrawDemoPanel()
{
    // --- Persistent demo state ---
    static ImGui::CustomTooltipConfig cfg{};
    static char tooltip_text[256] =
        "This is a custom tooltip.\nIt supports custom pivot, offset, padding, colors, and viewport clamping.";

    static bool enabled = true;
    static bool use_cursor_mode = true;   // pivot = (-1,-1)
    static bool show_button = true;
    static bool show_text = true;
    static bool show_disabled_item = true;

    if (use_cursor_mode)
        cfg.pivot = ImVec2(-1.0f, -1.0f);

    ImGui::TextUnformatted("Custom Tooltip Playground");
    ImGui::Separator();

    ImGui::BeginChild("tooltip_preview_child", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
    {
        ImGui::TextWrapped("Hover the preview items below to test the tooltip behavior.");

        if (show_text)
        {
            ImGui::Text("Hover this text");
            ImGui::DrawCustomTooltip(tooltip_text, cfg, enabled);
        }

        if (show_button)
        {
            ImGui::Button("Hover this button");
            ImGui::DrawCustomTooltip(tooltip_text, cfg, enabled);
        }

        if (show_disabled_item)
        {
            ImGui::BeginDisabled();
            ImGui::Button("Disabled item");
            ImGui::EndDisabled();
            ImGui::DrawCustomTooltip(tooltip_text, cfg, enabled);
        }

        ImGui::Spacing();
        ImGui::Separator();

        const bool cursor_mode = (cfg.pivot.x < 0.0f && cfg.pivot.y < 0.0f);
        ImGui::Text("Enabled: %s | Mode: %s | Clamp: %s | HoverFlags: 0x%X",
            enabled ? "Yes" : "No",
            cursor_mode ? "cursor" : "item pivot",
            cfg.keepWithinWorkArea ? "On" : "Off",
            cfg.hoverFlags);
        ImGui::Text("Pivot: (%.2f, %.2f) | Cursor Pivot: (%.2f, %.2f) | Offset: (%.1f, %.1f)",
            cfg.pivot.x, cfg.pivot.y,
            cfg.cursorPivot.x, cfg.cursorPivot.y,
            cfg.offset.x, cfg.offset.y);
        ImGui::Text("Padding: (%.1f, %.1f) | Rounding: %.1f",
            cfg.padding.x, cfg.padding.y, cfg.rounding);
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::SeparatorText("Settings");

    if (ImGui::BeginTable("tooltip_cfg_table", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 170.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        // =========================
        // CONTENT / STATE
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Tooltip Text");
        ImGui::TableSetColumnIndex(1);
        ImGui::InputTextMultiline("##tooltip_text", tooltip_text, IM_ARRAYSIZE(tooltip_text), ImVec2(-FLT_MIN, 70.0f));

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Enabled");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##enabled", &enabled);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Preview Items");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("Text##show_text", &show_text);
        ImGui::SameLine();
        ImGui::Checkbox("Button##show_button", &show_button);
        ImGui::SameLine();
        ImGui::Checkbox("Disabled##show_disabled", &show_disabled_item);

        // =========================
        // POSITIONING
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Use Cursor Mode");
        ImGui::TableSetColumnIndex(1);
        if (ImGui::Checkbox("##use_cursor_mode", &use_cursor_mode))
        {
            if (use_cursor_mode)
                cfg.pivot = ImVec2(-1.0f, -1.0f);
            else
                cfg.pivot = ImVec2(0.5f, 1.0f);
        }
        DrawHelpTooltip("ON = tooltip anchors from the mouse cursor.\nOFF = tooltip anchors from a normalized pivot on the hovered item.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Item Pivot");
        ImGui::TableSetColumnIndex(1);
        if (use_cursor_mode) ImGui::BeginDisabled();
        ImGui::SliderFloat2("##pivot", &cfg.pivot.x, 0.0f, 1.0f, "%.2f");
        if (use_cursor_mode) ImGui::EndDisabled();
        DrawHelpTooltip("Normalized anchor on the hovered item.\n(0,0)=top-left, (0.5,0.5)=center, (1,1)=bottom-right.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Cursor Pivot");
        ImGui::TableSetColumnIndex(1);
        ImGui::SliderFloat2("##cursor_pivot", &cfg.cursorPivot.x, 0.0f, 1.0f, "%.2f");
        DrawHelpTooltip("Pivot used by the tooltip window when in cursor mode.\nUseful to place the tooltip above, below, or beside the cursor.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Offset");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat2("##offset", &cfg.offset.x, 1.0f, -500.0f, 500.0f, "%.1f");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Keep In Work Area");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##keep_in_work_area", &cfg.keepWithinWorkArea);
        DrawHelpTooltip("Clamps the tooltip so it stays inside the viewport work area.");

        // =========================
        // APPEARANCE
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Padding");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat2("##padding", &cfg.padding.x, 0.25f, 0.0f, 50.0f, "%.1f");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Rounding");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat("##rounding", &cfg.rounding, 0.2f, -1.0f, 50.0f, "%.1f");
        DrawHelpTooltip("-1 uses the current style window rounding.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Background");
        ImGui::TableSetColumnIndex(1);
        DrawEditColorU32("##bg_col", &cfg.bgCol);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Border");
        ImGui::TableSetColumnIndex(1);
        DrawEditColorU32("##border_col", &cfg.borderCol);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Text Color");
        ImGui::TableSetColumnIndex(1);
        DrawEditColorU32("##text_col", &cfg.textCol);

        // =========================
        // HOVER FLAGS
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Allow Disabled");
        ImGui::TableSetColumnIndex(1);
        {
            bool allow_disabled = (cfg.hoverFlags & ImGuiHoveredFlags_AllowWhenDisabled) != 0;
            if (ImGui::Checkbox("##allow_disabled", &allow_disabled))
            {
                if (allow_disabled) cfg.hoverFlags |= ImGuiHoveredFlags_AllowWhenDisabled;
                else                cfg.hoverFlags &= ~ImGuiHoveredFlags_AllowWhenDisabled;
            }
        }
        DrawHelpTooltip("Allows tooltips to appear for disabled items.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Use DelayNormal");
        ImGui::TableSetColumnIndex(1);
        {
            bool delay_normal = (cfg.hoverFlags & ImGuiHoveredFlags_DelayNormal) != 0;
            if (ImGui::Checkbox("##delay_normal", &delay_normal))
            {
                if (delay_normal) cfg.hoverFlags |= ImGuiHoveredFlags_DelayNormal;
                else              cfg.hoverFlags &= ~ImGuiHoveredFlags_DelayNormal;
            }
        }
        DrawHelpTooltip("Applies Dear ImGui's normal hover delay before the tooltip appears.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("No Shared Delay");
        ImGui::TableSetColumnIndex(1);
        {
            bool no_shared_delay = (cfg.hoverFlags & ImGuiHoveredFlags_NoSharedDelay) != 0;
            if (ImGui::Checkbox("##no_shared_delay", &no_shared_delay))
            {
                if (no_shared_delay) cfg.hoverFlags |= ImGuiHoveredFlags_NoSharedDelay;
                else                 cfg.hoverFlags &= ~ImGuiHoveredFlags_NoSharedDelay;
            }
        }

        // =========================
        // RESET
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Reset");
        ImGui::TableSetColumnIndex(1);
        if (ImGui::SmallButton("Reset to defaults"))
        {
            cfg = ImGui::CustomTooltipConfig{};
            enabled = true;
            use_cursor_mode = true;
            show_button = true;
            show_text = true;
            show_disabled_item = true;

            snprintf(
                tooltip_text, IM_ARRAYSIZE(tooltip_text),
                "This is a custom tooltip.\nIt supports custom pivot, offset, padding, colors, and viewport clamping.");
        }

        ImGui::EndTable();
    }
}


