#pragma once
#include "imguiToggle.h"
#include "demo_module.h"
#include "easing.h"
#include <array>

class ToggleDemo : public DemoModule
{
	ImGui::ToggleConfig demo_cfg;
public:
	ToggleDemo() : DemoModule("Toggle", "Toggle Demo Panel") {}
protected:
	void DrawSelectedDemo();
    void OnPrePanel() override;
	void DrawDemoPanel() override;
};

static ImU32 StyleColU32(ImGuiCol idx, float alpha_mul = 1.0f)
{
	ImVec4 c = ImGui::GetStyleColorVec4(idx);
	c.w *= alpha_mul;
	return ImGui::GetColorU32(c);
}

void ToggleDemo::DrawSelectedDemo()
{
	ImGui::Text("Toggle Showcase");
	ImGui::Separator();

    const float line_width = 220.0f;

    // ------------------------------------------------------------
    // 1) Default (your stock toggle with default config)
    // ------------------------------------------------------------
    ImGui::Text("Default");
    ImGui::SameLine(line_width);
    static bool t_default = false;
    ImGui::Toggle("##toggle_default", &t_default);

    // ------------------------------------------------------------
    // 2) ImGui Dark Style
    //    - Colors derived from current ImGui style (Dark theme assumed)
    //    - Subtle rounding, same animation speed as default
    // ------------------------------------------------------------
    {
        ImGui::ToggleConfig cfg;
        // Geometry
        cfg.size = ImVec2(48, 22);
        cfg.rounding = 11.0f;       // pill track
        cfg.handle_rounding = 9.0f;        // rounded handle
        cfg.padding = 2.0f;        // normal handle spacing

        // Colors from style (feels "native" to ImGui Dark)
        // Off = FrameBg variants, On = SliderGrabActive hue for accent
        const ImU32 frame = StyleColU32(ImGuiCol_FrameBg);
        const ImU32 frameHov = StyleColU32(ImGuiCol_FrameBgHovered);
        const ImU32 grab = StyleColU32(ImGuiCol_SliderGrabActive);
        const ImU32 grabHov = StyleColU32(ImGuiCol_SliderGrabActive, 1.15f);

        cfg.col_off_bg = frame;
        cfg.col_off_hover_bg = frameHov;
        cfg.col_on_bg = grab;      // use accent for "on" track
        cfg.col_on_hover_bg = grabHov;

        // Handles: keep them light for contrast
        cfg.col_off_hnd = IM_COL32(235, 235, 235, 255);
        cfg.col_off_hover_hnd = IM_COL32(250, 250, 250, 255);
        cfg.col_on_hnd = IM_COL32(255, 255, 255, 255);
        cfg.col_on_hover_hnd = IM_COL32(255, 255, 255, 255);

        // Animation
        cfg.easingFunc = Easing::easeInOutQuad;
        cfg.anim_speed = 0.16f;

        ImGui::Text("ImGui Dark Style");
        ImGui::SameLine(line_width);
        static bool t_imgui = true;
        ImGui::Toggle("##toggle_imguidark", &t_imgui, cfg);
    }

    // ------------------------------------------------------------
    // 3) Material Purple (Dark Mode) — MD3-inspired
    //    Spec reference for vibe: https://m3.material.io/components/switch/specs
    //    Notes:
    //      - Track ON uses a purple primary (MD3 dark scheme: ~#6750A4)
    //      - Handle ON is light “onPrimaryContainer” feel (~#EADDFF)
    //      - Track OFF uses dark neutral (~#313033), hover slightly lighter
    //      - Elastic easing gives a lively snap
    // ------------------------------------------------------------
    {
        ImGui::ToggleConfig cfg;
        // Geometry (a bit taller / softer)
        cfg.size = ImVec2(60, 30);
        cfg.rounding = 15.0f;
        cfg.handle_rounding = 15.0f;
        cfg.padding = 2.0f;

        // Track (ON/OFF)
        cfg.col_on_bg = IM_COL32(0x67, 0x50, 0xA4, 255); // ~#6750A4
        cfg.col_on_hover_bg = IM_COL32(0x7F, 0x67, 0xBE, 255); // ~#7F67BE
        cfg.col_off_bg = IM_COL32(0x31, 0x30, 0x33, 255); // ~#313033
        cfg.col_off_hover_bg = IM_COL32(0x49, 0x45, 0x4F, 255); // ~#49454F

        // Handle (ON/OFF)
        cfg.col_on_hnd = IM_COL32(0xEA, 0xDD, 0xFF, 255); // ~#EADDFF
        cfg.col_on_hover_hnd = IM_COL32(0xF3, 0xEC, 0xFF, 255); // slightly lighter
        cfg.col_off_hnd = IM_COL32(0xE6, 0xE1, 0xE5, 255); // ~#E6E1E5
        cfg.col_off_hover_hnd = IM_COL32(0xF0, 0xEC, 0xF1, 255); // hover

        // Animation (elastic, per request)
        cfg.easingFunc = Easing::easeOutCubic;
        cfg.anim_speed = 0.28f;

        ImGui::Text("Material Purple (Dark Mode)");
        ImGui::SameLine(line_width);
        static bool t_md3 = false;
        ImGui::Toggle("##toggle_md3_purple_dark", &t_md3, cfg);
    }

    // ------------------------------------------------------------
    // 4) iOS Style
    //    - Bright green ON track
    //    - Soft pill with fully round handle
    //    - Elastic easing for that bouncy feel
    // ------------------------------------------------------------
    {
        ImGui::ToggleConfig cfg;
        cfg.size = ImVec2(60, 30);
        cfg.rounding = 15.0f;
        cfg.handle_rounding = 15.0f;
        cfg.padding = 2.0f;

        // Colors: iOS-like
        cfg.col_on_bg = IM_COL32(52, 199, 89, 255); // green
        cfg.col_on_hover_bg = IM_COL32(72, 220, 110, 255);
        cfg.col_off_bg = IM_COL32(174, 174, 178, 255);
        cfg.col_off_hover_bg = IM_COL32(200, 200, 204, 255);
        cfg.col_on_hnd = IM_COL32(255, 255, 255, 255);
        cfg.col_on_hover_hnd = IM_COL32(255, 255, 255, 255);
        cfg.col_off_hnd = IM_COL32(240, 240, 240, 255);
        cfg.col_off_hover_hnd = IM_COL32(250, 250, 250, 255);

        cfg.easingFunc = Easing::easeOutElastic; // per request
        cfg.anim_speed = 0.35f;

        ImGui::Text("iOS Style (Elastic)");
        ImGui::SameLine(line_width);
        static bool t_ios = true;
        ImGui::Toggle("##toggle_ios_elastic", &t_ios, cfg);
    }

    // ------------------------------------------------------------
    // 5) Square Flat (Minecraft-like)
    //    - Square handle bigger than the track (visual overlap)
    //    - 0 animation time: instant click
    //    - We achieve “larger handle than track” by using a small track height
    //      and slightly negative padding so the handle visually overflows.
    // ------------------------------------------------------------
    {
		ImGui::Dummy(ImVec2(0, 4)); // spacer
        ImGui::ToggleConfig cfg;
        // Geometry: skinny track + chunky handle
        cfg.size = ImVec2(44, 14);   // thin track
        cfg.rounding = 0.0f;             // square track
        cfg.handle_rounding = 0.0f;             // square handle
        cfg.padding = -2.0f;            // NEGATIVE padding -> handle taller than track (overlaps)

        // Instant click (no tween). Any easing is ignored effectively if speed == 0.
        cfg.anim_speed = 0.0f;

        ImGui::Text("Square Flat (Instant)");
        ImGui::SameLine(line_width);
        static bool t_flat = false;
        ImGui::Toggle("##toggle_square_flat", &t_flat, cfg);
        ImGui::Dummy(ImVec2(0, 4)); // spacer
    }
}

void ToggleDemo::OnPrePanel()
{
    ImGui::SetNextWindowSize({ 700, 600 }, ImGuiCond_Appearing);
}

// Optional: show the *effective* geometry with current config + style
static void ShowDerivedGeometryPreview(const ImGui::ToggleConfig& cfg)
{
    float frame_h = ImGui::GetFrameHeight();
    float h = (cfg.size.y > 0.0f) ? cfg.size.y : frame_h;
    float w = (cfg.size.x > 0.0f) ? cfg.size.x : (h * 2.0f);
    float track_round = (cfg.rounding >= 0.0f) ? cfg.rounding : (h * 0.5f);

    float handle_half = (h * 0.5f) - cfg.padding;
    float handle_round = (cfg.handle_rounding >= 0.0f) ? cfg.handle_rounding : handle_half;

    ImGui::Text("Effective track:  W=%.1f  H=%.1f  R=%.1f", w, h, track_round);
    ImGui::Text("Effective handle: W=%.1f  H=%.1f  R=%.1f",
        handle_half * 2.0f, (h - cfg.padding * 2.0f), handle_round);
}

void ToggleDemo::DrawDemoPanel()
{
    auto EditColorU32 = [](const char* label, ImU32* c) -> bool
    {
        ImVec4 v = ImGui::ColorConvertU32ToFloat4(*c);
        bool changed = ImGui::ColorEdit4(label, &v.x,
            ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
        if (changed) *c = ImGui::GetColorU32(v);
        return changed;
	};

    auto HelpMarker = [](const char* desc)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 40.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
	};

    // Live preview value
    static bool demo_val = true;

    // Layout: left = controls, right = preview
    if (ImGui::BeginTable("toggle_cfg_table", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Config", ImGuiTableColumnFlags_WidthStretch, 0.60f);
        ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthStretch, 0.40f);
        ImGui::TableNextRow();

        // =========================
        // Left: controls
        // =========================
        ImGui::TableSetColumnIndex(0);

        // Header + Reset
        ImGui::TextUnformatted("Toggle Config (playground)");
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset to defaults"))
        {
            demo_cfg = ImGui::ToggleConfig{}; // value-init uses your defaults from imguiToggle.h
        }

        ImGui::SeparatorText("Geometry");
        {
            // Size: 0 means auto (w=2*h, h=frame height)
            ImGui::TextUnformatted("Size");
            HelpMarker("Set to (0,0) for auto: height = FrameHeight, width = 2 * height.");
            ImGui::DragFloat2("size (w,h)", &demo_cfg.size.x, 1.0f, 0.0f, 1000.0f, "%.1f");

            // Rounding: -1 means auto (track: half-height; handle: circle)
            ImGui::TextUnformatted("Rounding");
            HelpMarker("Track rounding: -1 = half-height.\nHandle rounding: -1 = circle.");
            ImGui::DragFloat("track rounding", &demo_cfg.rounding, 0.2f, -1.0f, 100.0f, "%.1f");
            ImGui::DragFloat("handle rounding", &demo_cfg.handle_rounding, 0.2f, -1.0f, 100.0f, "%.1f");

            // Padding between handle & track box
            ImGui::DragFloat("padding", &demo_cfg.padding, 0.1f, -8.0f, 16.0f, "%.2f");
            HelpMarker("Space between handle and track box.\nCan be negative for a larger handle look (e.g., Minecraft-like).");

            if (ImGui::TreeNode("Derived (read-only)"))
            {
                ShowDerivedGeometryPreview(demo_cfg);
                ImGui::TreePop();
            }
        }

        ImGui::SeparatorText("Track Colors");
        {
            EditColorU32("off bg", &demo_cfg.col_off_bg);
            EditColorU32("off hover bg", &demo_cfg.col_off_hover_bg);
            EditColorU32("on bg", &demo_cfg.col_on_bg);
            EditColorU32("on hover bg", &demo_cfg.col_on_hover_bg);
        }

        ImGui::SeparatorText("Handle Colors");
        {
            EditColorU32("off handle", &demo_cfg.col_off_hnd);
            EditColorU32("off hover handle", &demo_cfg.col_off_hover_hnd);
            EditColorU32("on handle", &demo_cfg.col_on_hnd);
            EditColorU32("on hover handle", &demo_cfg.col_on_hover_hnd);
        }

        ImGui::SeparatorText("Animation");
        {
            int ease_idx = IndexFromEasing(demo_cfg.easingFunc);
            if (ImGui::BeginCombo("Easing", EASING_FNS[ease_idx].name))
            {
                for (int i = 0; i < (int)EASING_FNS_COUNT; ++i)
                {
                    bool selected = (i == ease_idx);
                    if (ImGui::Selectable(EASING_FNS[i].name, selected))
                    {
                        demo_cfg.easingFunc = EASING_FNS[i].fn;
                        ease_idx = i;
                    }
                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::DragFloat("anim speed (sec full)", &demo_cfg.anim_speed, 0.01f, 0.0f, 2.0f, "%.2f");
            HelpMarker("Time (in seconds) to go from 0.0 -> 1.0.\nSet to 0.0 for an instant click (no tween).");
        }

        // =========================
        // Right: live preview
        // =========================
        ImGui::TableSetColumnIndex(1);

        ImGui::TextUnformatted("Preview");
        ImGui::Separator();

        // Optional framing so users can see hover states clearly
        ImGui::BeginChild("preview_child", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
        {
            // Draw a label + live toggle with current config
            ImGui::Text("demo_toggle");
            ImGui::SameLine(180.0f);

            // The preview toggle
            ImGui::Toggle("##demo_toggle", &demo_val, demo_cfg);

            // Status line (helps debug)
            ImGui::Text("state: %s | anim: %s | easing: %s",
                demo_val ? "ON" : "OFF",
                (demo_cfg.anim_speed <= 0.0f ? "instant" : "smooth"),
                EASING_FNS[IndexFromEasing(demo_cfg.easingFunc)].name);
        }
        ImGui::EndChild();

        ImGui::EndTable();
    }
}


