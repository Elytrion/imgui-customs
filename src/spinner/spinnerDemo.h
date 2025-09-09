#pragma once
#include "imguiSpinner.h"
#include "demo_module.h"

class SpinnerDemo : public DemoModule
{
    ImGui::SpinnerConfig demo_cfg;
    bool enable_custom_overlay = false;
    int  overlay_style = 1; // 0: None, 1: Center Dot, 2: Crosshair, 3: Radial Ticks, 4: Center Text
public:
	SpinnerDemo() : DemoModule("Custom Spinner", "Spinner Demo Panel") {}
protected:
	void DrawSelectedDemo();
    void OnPrePanel() override;
	void DrawDemoPanel() override;
};

inline void SpinnerDemo::DrawSelectedDemo()
{
    ImGui::Text("Spinner Showcase");
    ImGui::Separator();

    const float line_width = 250.0f;

    // 1. Default material spinner (track visible)
    ImGui::Text("Default (Material):");
    ImGui::SameLine(line_width);
    ImGui::Spinner("##default");

    // 2. Material spinner, no track
    {
        ImGui::SpinnerConfig cfg;
        cfg.show_track = false;
        ImGui::Text("Material, No Track:");
        ImGui::SameLine(line_width);
        ImGui::Spinner("##no_track", cfg);
    }

    // 3. Constant-speed, variable arc (still grows/shrinks)
    {
        ImGui::SpinnerConfig cfg;
        cfg.period = 3.0f;
        cfg.constant_speed = true;
        cfg.constant_arc = false;
        cfg.indicator_color = IM_COL32(100, 220, 255, 255);
        ImGui::Text("Constant Speed, Variable Arc:");
        ImGui::SameLine(line_width);
        ImGui::Spinner("##const_speed_var_arc", cfg);
    }

    // 4. Constant-speed, fixed arc (never grows/shrinks)
    {
        ImGui::SpinnerConfig cfg;
        cfg.period = 3.0f;
        cfg.constant_speed = true;
        cfg.constant_arc = true;
        cfg.arc_length_deg = 100.0f;
        cfg.indicator_color = IM_COL32(255, 180, 50, 255);
        ImGui::Text("Constant Speed, Fixed Arc:");
        ImGui::SameLine(line_width);
        ImGui::Spinner("##const_speed_fixed_arc", cfg);
    }

    // 5. Material mode with track thickness emphasized
    {
        ImGui::SpinnerConfig cfg;
        cfg.track_thickness = 5.0f;
        cfg.indicator_thickness = 2.0f;
        cfg.track_color = IM_COL32(100, 180, 100, 100);
        cfg.indicator_color = IM_COL32(100, 200, 100, 255);
        cfg.period = 3.0f;
        cfg.constant_speed = true;
        cfg.constant_arc = true;
        cfg.arc_length_deg = 100.0f;
        ImGui::Text("Constant with Thick Track:");
        ImGui::SameLine(line_width);
        ImGui::Spinner("##thick_track", cfg);
    }
}

inline void SpinnerDemo::OnPrePanel()
{
    ImGui::SetNextWindowSize({ 600.0f, 550.0f }, ImGuiCond_Appearing);
}

inline void SpinnerDemo::DrawDemoPanel()
{
    // Small helpers to manipulate ImU32 with ColorEdit (float RGBA)
    auto U32ToVec4 = [](ImU32 c) { return ImGui::ColorConvertU32ToFloat4(c); };
    auto Vec4ToU32 = [](ImVec4 v) { return ImGui::ColorConvertFloat4ToU32(v); };

    ImGui::TextUnformatted("Custom Spinner");
    ImGui::Separator();

    // ----- Preview (always shown) -----
    // If draw_fn is enabled below, it will be attached here.
    ImGui::Spinner("##preview_custom_spinner", demo_cfg);

    ImGui::Spacing();
    ImGui::SeparatorText("Settings");
    ImGui::Spacing();

    // ----- Layout: two columns (labels | widgets) -----
    if (ImGui::BeginTable("spinner_cfg_table", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 160.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        // ====== SIZE & THICKNESS ======
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Radius");
        ImGui::TableSetColumnIndex(1); ImGui::SliderFloat("##radius", &demo_cfg.radius, 1.0f, 100.0f, "%.1f");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Indicator Thickness");
        ImGui::TableSetColumnIndex(1); ImGui::SliderFloat("##indicator_th", &demo_cfg.indicator_thickness, 1.0f, 30.0f, "%.1f");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Indicator Color");
        ImGui::TableSetColumnIndex(1);
        {
            ImVec4 col = U32ToVec4(demo_cfg.indicator_color);
            if (ImGui::ColorEdit4("##indicator_col", &col.x, ImGuiColorEditFlags_NoInputs))
                demo_cfg.indicator_color = Vec4ToU32(col);
        }

        // ====== TRACK ======
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Show Track");
        ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##show_track", &demo_cfg.show_track);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Track Thickness");
        ImGui::TableSetColumnIndex(1);
        ImGui::SliderFloat("##track_th", &demo_cfg.track_thickness, 1.0f, 30.0f, "%.1f");
        if (!demo_cfg.show_track) ImGui::BeginDisabled();
        ImGui::SameLine();
        {
            ImVec4 col = U32ToVec4(demo_cfg.track_color);
            if (ImGui::ColorEdit4("Track Color", &col.x, ImGuiColorEditFlags_NoInputs))
                demo_cfg.track_color = Vec4ToU32(col);
        }
        if (!demo_cfg.show_track) ImGui::EndDisabled();

        // ====== MOTION MODE ======
        // Present three canonical modes via a combo to toggle boolean pair {constant_speed, constant_arc}
        int mode = 0; // 0: Material, 1: ConstantSpeed+VariableArc, 2: ConstantSpeed+FixedArc
        if (demo_cfg.constant_speed && !demo_cfg.constant_arc) mode = 1;
        if (demo_cfg.constant_speed && demo_cfg.constant_arc)  mode = 2;

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Motion Mode");
        ImGui::TableSetColumnIndex(1);
        if (ImGui::BeginCombo("##mode_combo",
            mode == 0 ? "Material (default)"
            : mode == 1 ? "Constant Speed + Variable Arc"
            : "Constant Speed + Fixed Arc"))
        {
            if (ImGui::Selectable("Material (default)", mode == 0)) { mode = 0; }
            if (ImGui::Selectable("Constant Speed + Variable Arc", mode == 1)) { mode = 1; }
            if (ImGui::Selectable("Constant Speed + Fixed Arc", mode == 2)) { mode = 2; }
            ImGui::EndCombo();
        }
        // Apply chosen mode to config booleans
        demo_cfg.constant_speed = (mode != 0);
        demo_cfg.constant_arc = (mode == 2);

        // ====== PERIOD (speed) ======
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Period (sec)");
        ImGui::TableSetColumnIndex(1);
        ImGui::SliderFloat("##period", &demo_cfg.period, 0.25f, 10.0f, "%.2f");

        // ====== ARC SETTINGS ======
        // Material/variable-arc fields:
        bool var_arc_enabled = (mode != 2); // not fixed-arc
        if (!var_arc_enabled) ImGui::BeginDisabled();
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Min Arc (deg)");
        ImGui::TableSetColumnIndex(1);
        ImGui::SliderFloat("##min_arc", &demo_cfg.min_arc, 0.0f, 360.0f, "%.0f");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Max Arc (deg)");
        ImGui::TableSetColumnIndex(1);
        ImGui::SliderFloat("##max_arc", &demo_cfg.max_arc, 0.0f, 360.0f, "%.0f");
        if (!var_arc_enabled) ImGui::EndDisabled();

        // Fixed-arc field:
        bool fixed_arc_enabled = (mode == 2);
        if (!fixed_arc_enabled) ImGui::BeginDisabled();
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Fixed Arc Length (deg)");
        ImGui::TableSetColumnIndex(1);
        ImGui::SliderFloat("##arc_len_deg", &demo_cfg.arc_length_deg, 1.0f, 359.0f, "%.0f");
        if (!fixed_arc_enabled) ImGui::EndDisabled();

        // ====== MATERIAL-STYLE ONLY ======
        bool material_enabled = (mode == 0);
        if (!material_enabled) ImGui::BeginDisabled();
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Detents");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragInt("##detents", &demo_cfg.detents, 0.1f, 1, 16);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Skip Detents");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragInt("##skip_detents", &demo_cfg.skip_detents, 0.1f, 0, 16);
        if (!material_enabled) ImGui::EndDisabled();

        // ====== CUSTOM DRAW FN ======
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Custom Overlay");
        ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##enable_overlay", &enable_custom_overlay);

        if (!enable_custom_overlay) ImGui::BeginDisabled();
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Overlay Style");
        ImGui::TableSetColumnIndex(1);
        if (ImGui::BeginCombo("##overlay_style",
            overlay_style == 0 ? "None" :
            overlay_style == 1 ? "Center Dot" :
            overlay_style == 2 ? "Crosshair" :
            overlay_style == 3 ? "Radial Ticks" :
            "Center Text"))
        {
            if (ImGui::Selectable("None", overlay_style == 0)) overlay_style = 0;
            if (ImGui::Selectable("Center Dot", overlay_style == 1)) overlay_style = 1;
            if (ImGui::Selectable("Crosshair", overlay_style == 2)) overlay_style = 2;
            if (ImGui::Selectable("Radial Ticks", overlay_style == 3)) overlay_style = 3;
            if (ImGui::Selectable("Center Text", overlay_style == 4)) overlay_style = 4;
            ImGui::EndCombo();
        }
        if (!enable_custom_overlay) ImGui::EndDisabled();

        ImGui::EndTable();

        // Attach / detach draw_fn based on UI
        if (!enable_custom_overlay || overlay_style == 0)
        {
            demo_cfg.draw_fn = nullptr;
        }
        else
        {
            // NOTE: draw_fn receives the spinner center; we can pull DrawList from current window.
            demo_cfg.draw_fn = [this](ImVec2 c)
                {
                    ImDrawList* dl = ImGui::GetCurrentWindow()->DrawList;

                    switch (overlay_style)
                    {
                    case 1: // Center Dot
                    {
                        dl->AddCircleFilled(c, ImMax(1.0f, demo_cfg.indicator_thickness * 0.5f), demo_cfg.indicator_color, 12);
                    } break;

                    case 2: // Crosshair
                    {
                        float r = demo_cfg.radius * 0.6f;
                        float t = ImMax(1.0f, demo_cfg.track_thickness > 0 ? demo_cfg.track_thickness : 1.5f);
                        dl->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x + r, c.y), demo_cfg.track_color, t);
                        dl->AddLine(ImVec2(c.x, c.y - r), ImVec2(c.x, c.y + r), demo_cfg.track_color, t);
                    } break;

                    case 3: // Radial Ticks (12 small marks on the track)
                    {
                        const int N = 12;
                        float r0 = demo_cfg.radius - demo_cfg.indicator_thickness * 0.5f;
                        float r1 = r0 - ImMax(1.0f, demo_cfg.track_thickness) * 2.0f;
                        ImU32 col = demo_cfg.track_color ? demo_cfg.track_color : IM_COL32(255, 255, 255, 96);
                        for (int i = 0; i < N; ++i)
                        {
                            float a = (2.0f * IM_PI) * (float)i / (float)N;
                            ImVec2 p0 = ImVec2(c.x + r0 * cosf(a), c.y + r0 * sinf(a));
                            ImVec2 p1 = ImVec2(c.x + r1 * cosf(a), c.y + r1 * sinf(a));
                            dl->AddLine(p0, p1, col, 1.0f);
                        }
                    } break;

                    case 4: // Center Text (tiny)
                    {
                        // Draw a small text “Hi” at the center with outline for readability
                        const char* txt = "Hello World";
                        ImVec2 ts = ImGui::CalcTextSize(txt);
                        ImVec2 p = ImVec2(c.x - ts.x * 0.5f, c.y - ts.y * 0.5f);
                        ImU32 col = demo_cfg.indicator_color;
                        ImU32 outline = IM_COL32(0, 0, 0, 160);
                        dl->AddText(ImVec2(p.x + 1, p.y + 1), outline, txt);
                        dl->AddText(p, col, txt);
                    } break;
                    }
                };
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextWrapped("Tip: Motion + arc math follows the Material-style logic unless you pick "
        "\"Constant Speed + Fixed Arc\" (never grows/shrinks).");
}


