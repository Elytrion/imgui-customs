#pragma once
#include "imguiMultiToggle.h"
#include "demo_module.h"
#include "easing.h"

class MultiToggleDemo : public DemoModule
{
public:
	MultiToggleDemo() : DemoModule("Multi-Toggle", "Multi-Toggle Demo Panel") {}
protected:
	void DrawSelectedDemo();
    void OnPrePanel() override;
	void DrawDemoPanel() override;
};

void MultiToggleDemo::DrawSelectedDemo()
{
	ImGui::TextWrapped("This is a demo of the MultiToggle widget, which allows selection among multiple options with smooth animations and customizable styles.");
	static int current = 0;
	bool changed = ImGui::MultiToggle("##multi", &current, { "Option 1", "Option 2", "Option 3", "Option 4" });
	ImGui::Text("Current selection: Option %d", current + 1);
}

void MultiToggleDemo::OnPrePanel()
{
    ImGui::SetNextWindowSize({ 700, 600 }, ImGuiCond_Appearing);
}

void MultiToggleDemo::DrawDemoPanel()
{    
    auto EditColorU32 = [](const char* label, ImU32* c) {
        ImVec4 v = ImGui::ColorConvertU32ToFloat4(*c);
        bool changed = ImGui::ColorEdit4(label, &v.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
        if (changed) *c = ImGui::GetColorU32(v);
        return changed;
        };
    auto HelpTooltip = [](const char* text) {
        ImGui::SameLine(); ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 42.0f);
            ImGui::TextUnformatted(text);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
        };

    // --- Persistent demo state ---
    static ImGui::MultiToggleConfig cfg;               // UI-configurable
    static int current_index = 0;                      // selected entry
    static int option_count = 4;                      // 2..8
    static std::vector<std::array<char, 64>> labels;   // editable labels (buffers)

    // Bootstrap labels (once)
    if (labels.empty()) {
        labels.resize(option_count);
        for (int i = 0; i < option_count; ++i)
            snprintf(labels[i].data(), labels[i].size(), "Option %d", i + 1);
    }

    ImGui::TextUnformatted("Multi-Toggle Playground");
    ImGui::Separator();

    ImGui::BeginChild("multi_preview_child", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
    {
        std::vector<const char*> cstr;
        cstr.reserve(labels.size());
        for (auto& b : labels) cstr.push_back(b.data());

        bool changed = ImGui::MultiToggle("##multi_preview_again", &current_index, cstr, cfg);
        ImGui::Text("Changed this frame: %s", changed ? "Yes" : "No ");
        ImGui::SameLine();
        ImGui::Text("| Current selection: %s", cstr[current_index]);
        ImGui::Text("Index: %d | Anim: %s | Sections: %s | Gap: %.1f",
            current_index,
            (cfg.anim_speed <= 0.0f ? "instant" : "smooth"),
            cfg.equal_sections ? "equal" : "unequal",
            cfg.equal_sections ? 0.0f : cfg.gap);
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::SeparatorText("Settings");

    // Two-column layout (labels | widgets)
    if (ImGui::BeginTable("multi_cfg_table", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 170.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        // =========================
        // OPTIONS (count + names)
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Options");
        ImGui::TableSetColumnIndex(1);
        if (ImGui::SliderInt("##opt_count", &option_count, 2, 8)) {
            if (option_count > (int)labels.size()) {
                int add = option_count - (int)labels.size();
                for (int k = 0; k < add; ++k) {
                    std::array<char, 64> buf{};
                    snprintf(buf.data(), buf.size(), "Option %d", (int)labels.size() + 1);
                    labels.push_back(buf);
                }
            }
            else if (option_count < (int)labels.size()) {
                labels.resize(option_count);
                current_index = ImClamp(current_index, 0, option_count - 1);
            }
        }

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Names");
        ImGui::TableSetColumnIndex(1);
        for (int i = 0; i < option_count; ++i) {
            ImGui::PushID(i);
            ImGui::InputText("##name", labels[i].data(), labels[i].size());
            ImGui::SameLine();
            ImGui::TextDisabled("(%d)", i + 1);
            ImGui::PopID();
        }

        // =========================
        // GEOMETRY
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Size (W,H)");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat2("##size", &cfg.size.x, 1.0f, 0.0f, 2000.0f, "%.1f");
        HelpTooltip("Set (0,0) for auto height = 1.35 * FrameHeight and width fills content region.\n"
            "When unequal sections are enabled, width also accounts for gaps and measured text.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Track Rounding");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat("##rounding", &cfg.rounding, 0.2f, -1.0f, 200.0f, "%.1f");
        HelpTooltip("-1 = half height (pill).");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Plate Padding");
        ImGui::TableSetColumnIndex(1);
        ImGui::SliderFloat("##plate_pad", &cfg.plate_pad, 0.0f, 24.0f, "%.1f");
        HelpTooltip("Inner padding around the moving plate (left/right & top/bottom).");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Equal Sections");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##equal", &cfg.equal_sections);
        HelpTooltip("ON = evenly split track width into N sections.\nOFF = section width from measured text.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Gap (unequal only)");
        ImGui::TableSetColumnIndex(1);
        if (cfg.equal_sections) ImGui::BeginDisabled();
        ImGui::SliderFloat("##gap", &cfg.gap, 0.0f, 40.0f, "%.1f");
        if (cfg.equal_sections) ImGui::EndDisabled();

        // =========================
        // COLORS
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Track Color");
        ImGui::TableSetColumnIndex(1); EditColorU32("##track_col", &cfg.col_track);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Track Hover");
        ImGui::TableSetColumnIndex(1); EditColorU32("##track_hover", &cfg.col_track_hover);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Plate Color");
        ImGui::TableSetColumnIndex(1); EditColorU32("##plate_col", &cfg.col_plate);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Plate Hover");
        ImGui::TableSetColumnIndex(1); EditColorU32("##plate_hover", &cfg.col_plate_hover);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Text Inactive");
        ImGui::TableSetColumnIndex(1); EditColorU32("##text_inactive", &cfg.col_text);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Text Active");
        ImGui::TableSetColumnIndex(1); EditColorU32("##text_active", &cfg.col_text_active);

        // =========================
        // ANIMATION
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Anim Speed (sec)");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat("##anim_speed", &cfg.anim_speed, 0.01f, 0.0f, 2.0f, "%.2f");
        HelpTooltip("Time to traverse fully from one option to another.\n0.0 = instant snap.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Easing");
        ImGui::TableSetColumnIndex(1);
        {
            int idx = IndexFromEasing(cfg.easing);
            if (ImGui::BeginCombo("##easing_combo", EASING_FNS[idx].name)) {
                for (int i = 0; i < (int)EASING_FNS_COUNT; ++i) {
                    bool sel = (i == idx);
                    if (ImGui::Selectable(EASING_FNS[i].name, sel)) { cfg.easing = EASING_FNS[i].fn; idx = i; }
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        // =========================
        // CURRENT INDEX (optional)
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Current Index");
        ImGui::TableSetColumnIndex(1);
        if (ImGui::SliderInt("##cur_idx", &current_index, 0, option_count - 1)) {
            // NOTE: MultiToggle animates only on click; direct index changes snap logically.
            current_index = ImClamp(current_index, 0, option_count - 1);
        }

        // =========================
        // RESET
        // =========================
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Reset");
        ImGui::TableSetColumnIndex(1);
        if (ImGui::SmallButton("Reset to defaults")) {
            cfg = ImGui::MultiToggleConfig{};          // struct defaults
            option_count = 4;
            labels.clear();
            labels.resize(option_count);
            for (int i = 0; i < option_count; ++i)
                snprintf(labels[i].data(), labels[i].size(), "Option %d", i + 1);
            current_index = 0;
        }

        ImGui::EndTable();
    }
}




