#pragma once
#include "imguiTween.h"
#include "demo_module.h"

class TweenDemo : public DemoModule
{
public:
	TweenDemo() : DemoModule("Imgui Tween", "Tween Demo Panel") {}
protected:
	void DrawSelectedDemo();
    void OnPrePanel() override;
	void DrawDemoPanel() override;
};

static void TD_Help(const char* text)
{
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 42.0f);
        ImGui::TextUnformatted(text);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
struct TDEasingEntry { const char* name; float (*fn)(float); };
static constexpr TDEasingEntry TD_EASINGS[] = {
    { "Linear (none)", nullptr },
    { "Sine In",       Easing::easeInSine },
    { "Sine Out",      Easing::easeOutSine },
    { "Sine InOut",    Easing::easeInOutSine },
    { "Quad InOut",    Easing::easeInOutQuad },
    { "Cubic Out",     Easing::easeOutCubic },
    { "Cubic InOut",   Easing::easeInOutCubic },
    { "Quart Out",     Easing::easeOutQuart },
    { "Quint Out",     Easing::easeOutQuint },
    { "Expo InOut",    Easing::easeInOutExpo },
    { "Circ InOut",    Easing::easeInOutCirc },
    { "Back InOut",    Easing::easeInOutBack },
    { "Elastic Out",   Easing::easeOutElastic },
    { "Elastic InOut", Easing::easeInOutElastic },
    { "Bounce Out",    Easing::easeOutBounce },
    { "Bounce InOut",  Easing::easeInOutBounce },
};
static int TD_IndexFromEasing(float (*fn)(float))
{
    for (int i = 0; i < (int)IM_ARRAYSIZE(TD_EASINGS); ++i)
        if (TD_EASINGS[i].fn == fn) return i;
    return 0;
}

void TweenDemo::DrawSelectedDemo()
{
    // A single interactive “card” that tweens multiple properties on hover:
    // - background color (ImVec4)
    // - width & height (ImVec2)
    // - corner radius (float)
    // - soft shadow offset (ImVec2)
    //
    // Everything is driven by the same `inside` (hover) boolean but uses
    // independent IDs so tweens can run concurrently (per your Tween design).

    ImGui::TextUnformatted("Tween Showcase");
    ImGui::Separator();

    // Interaction surface
    const char* card_id = "##tween_card_demo";
    ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 base = ImVec2(180, 40);   // min size
    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Make a hittable area a bit larger than min size so hover is easy
    ImGui::InvisibleButton(card_id, ImVec2(base.x + 40, base.y + 10));
    bool hovered = ImGui::IsItemHovered();

    // Durations & easing for the mini demo (snappy)
    const float up = 0.16f;  // hover in
    const float down = 0.12f;  // hover out
    auto easing = Easing::easeOutCubic;

    // 1) Size tween (ImVec2)
    ImVec2 sz = ImGui::Tween<ImVec2>(
        "card.size", hovered, up, down,
        base, ImVec2(230, 50),
        ImGuiTweenFlags_StartMin, easing);

    // 2) Rounding tween (float)
    float rounding = ImGui::Tween<float>(
        "card.rounding", hovered, up, down,
        6.0f, 14.0f, ImGuiTweenFlags_StartMin, easing);

    // 3) Background color tween (ImVec4)
    ImVec4 col = ImGui::Tween<ImVec4>(
        "card.bg", hovered, up, down,
        ImVec4(0.15f, 0.15f, 0.17f, 1.00f),   // at rest
        ImVec4(0.45f, 0.25f, 0.80f, 1.00f),   // hover
        ImGuiTweenFlags_StartMin, easing);

    // 4) Shadow offset tween (ImVec2)
    ImVec2 sh = ImGui::Tween<ImVec2>(
        "card.shadow", hovered, up, down,
        ImVec2(0, 0), ImVec2(6, 6),
        ImGuiTweenFlags_StartMin, easing);

    // Draw shadow then card
    ImU32 shadow_col = ImGui::GetColorU32(ImVec4(0.5, 0.5, 0.5, 0.25f));
    dl->AddRectFilled(ImVec2(pos.x + sh.x, pos.y + sh.y),
        ImVec2(pos.x + sh.x + sz.x, pos.y + sh.y + sz.y),
        shadow_col, rounding);
    dl->AddRectFilled(pos, ImVec2(pos.x + sz.x, pos.y + sz.y),
        ImGui::GetColorU32(col), rounding);

    // Label
    ImVec2 text_sz = ImGui::CalcTextSize("Hover me");
    dl->AddText(ImVec2(pos.x + (sz.x - text_sz.x) * 0.5f,
        pos.y + (sz.y - text_sz.y) * 0.5f),
        ImGui::GetColorU32(ImGuiCol_Text), "Hover me");

    ImGui::Separator();
    ImGui::TextWrapped("This demo shows a simple interactive card that tweens multiple properties on hover. "
        "The tweens are all independent but share the same hover state. "
		"The code is in the TweenDemo::DrawSelectedDemo() function.");
}

void TweenDemo::OnPrePanel()
{
	ImGui::SetNextWindowSize(ImVec2(500, 620), ImGuiCond_Appearing);
}

void TweenDemo::DrawDemoPanel()
{
    // Live preview state
    static bool inside_manual = false;       // used when Drive = Manual
    static bool auto_loop = false;       // Drive = Auto (loop)
    static float auto_pause = 0.00f;       // extra dwell time at each endpoint
    static int   drive_mode = 0;           // 0=Hover, 1=Manual, 2=Auto

    // Tween params
    static float upDur = 0.20f;
    static float downDur = 0.18f;
    static int   easing_idx = TD_IndexFromEasing(Easing::easeInOutCubic);
    static float (*easeFn)(float) = TD_EASINGS[easing_idx].fn;

    // Channels (min/max)
    static bool ch_pos = true;   static float   pos_min = 0.0f, pos_max = 120.0f;  // X offset
    static bool ch_size = true;   static ImVec2  size_min = ImVec2(160, 38), size_max = ImVec2(240, 56);
    static bool ch_round = true;   static float   r_min = 4.0f, r_max = 16.0f;
    static bool ch_bg = true;   static ImVec4  bg_min = ImVec4(0.14f, 0.14f, 0.16f, 1.0f),
        bg_max = ImVec4(0.35f, 0.22f, 0.70f, 1.0f);
    static bool ch_text = false;  static float   a_min = 0.40f, a_max = 1.00f;   // text alpha

    ImGui::TextUnformatted("Tween Playground");
    ImGui::Separator();

    // ------------------------ PREVIEW TILE ------------------------
    ImGui::BeginChild("tween_preview", ImVec2(0, 170), ImGuiChildFlags_Border);
    {
        // Interaction surface
        const char* tile_id = "##tween_tile";
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImVec2 avail = ImGui::GetContentRegionAvail();

        // Hit area spans width, but we'll center our tile inside it.
        ImGui::InvisibleButton(tile_id, ImVec2(avail.x, 120.0f));
        bool hovered = ImGui::IsItemHovered();

        // Choose driver -> inside
        bool inside = false;
        if (drive_mode == 0) inside = hovered;               // Hover
        else if (drive_mode == 1) inside = inside_manual;         // Manual
        else
        {
            // Auto loop: inside true during [0, upDur], false during [upDur, upDur+downDur],
            // with an optional pause at each end.
            float t = (float)ImGui::GetTime();
            float period = ImMax(0.0001f, upDur + downDur + 2.0f * auto_pause);
            float phase = fmodf(t, period);
            inside = (phase <= (upDur + auto_pause));
        }

        // Compute tweens (independent IDs)
        ImVec2 sz = ImVec2(200, 48);
        if (ch_size)
            sz = ImGui::Tween<ImVec2>("tile.size", inside, upDur, downDur,
                size_min, size_max, ImGuiTweenFlags_StartMin, easeFn);

        float rounding = 8.0f;
        if (ch_round)
            rounding = ImGui::Tween<float>("tile.round", inside, upDur, downDur,
                r_min, r_max, ImGuiTweenFlags_StartMin, easeFn);

        float xoff = 0.0f;
        if (ch_pos)
            xoff = ImGui::Tween<float>("tile.x", inside, upDur, downDur,
                pos_min, pos_max, ImGuiTweenFlags_StartMin, easeFn);

        ImVec4 bg = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
        if (ch_bg)
            bg = ImGui::Tween<ImVec4>("tile.bg", inside, upDur, downDur,
                bg_min, bg_max, ImGuiTweenFlags_StartMin, easeFn);

        float a = 1.0f;
        if (ch_text)
            a = ImGui::Tween<float>("tile.txt", inside, upDur, downDur,
                a_min, a_max, ImGuiTweenFlags_StartMin, easeFn);

        // Center tile horizontally; apply x offset from tween
        float cx = p.x + (avail.x - sz.x) * 0.5f + xoff;
        float cy = p.y + 35.0f;

        // Shadow for depth
        dl->AddRectFilled(ImVec2(cx + 6, cy + 6), ImVec2(cx + sz.x + 6, cy + sz.y + 6),
            ImGui::GetColorU32(ImVec4(0, 0, 0, 0.22f)), rounding);

        // Tile
        dl->AddRectFilled(ImVec2(cx, cy), ImVec2(cx + sz.x, cy + sz.y),
            ImGui::GetColorU32(bg), rounding);

        // Label
        ImU32 txt_col = ImGui::GetColorU32(ImVec4(1, 1, 1, a));
        const char* label = inside ? "Inside -> Max" : "Outside -> Min";
        ImVec2 ts = ImGui::CalcTextSize(label);
        dl->AddText(ImVec2(cx + (sz.x - ts.x) * 0.5f, cy + (sz.y - ts.y) * 0.5f),
            txt_col, label);

        // Instruction line
        ImGui::SetCursorScreenPos(ImVec2(p.x, cy + sz.y + 16.0f));
        ImGui::TextDisabled("Drive: %s  |  Hover to test or change mode on the right",
            drive_mode == 0 ? "Hover"
            : drive_mode == 1 ? "Manual"
            : "Auto");
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::SeparatorText("Settings");

    // ------------------------ SETTINGS GRID ------------------------
    if (ImGui::BeginTable("tween_cfg_tbl", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 160.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        // DRIVE MODE
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Drive");
        ImGui::TableSetColumnIndex(1);
        ImGui::RadioButton("Hover", &drive_mode, 0); ImGui::SameLine();
        ImGui::RadioButton("Manual", &drive_mode, 1); ImGui::SameLine();
        ImGui::RadioButton("Auto", &drive_mode, 2);

        if (drive_mode == 1) {
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Manual Inside");
            ImGui::TableSetColumnIndex(1);
            ImGui::Checkbox("##inside", &inside_manual); TD_Help("Toggle to send the tween from min->max (true) or max->min (false).");
        }
        if (drive_mode == 2) {
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Auto Pause (s)");
            ImGui::TableSetColumnIndex(1);
            ImGui::DragFloat("##pause", &auto_pause, 0.01f, 0.0f, 2.0f, "%.2f");
        }

        // DURATIONS & EASING
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Up Duration (s)");
        ImGui::TableSetColumnIndex(1); ImGui::DragFloat("##up", &upDur, 0.01f, 0.0f, 2.0f, "%.2f");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Down Duration (s)");
        ImGui::TableSetColumnIndex(1); ImGui::DragFloat("##down", &downDur, 0.01f, 0.0f, 2.0f, "%.2f");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Easing");
        ImGui::TableSetColumnIndex(1);
        {
            if (ImGui::BeginCombo("##ease", TD_EASINGS[easing_idx].name)) {
                for (int i = 0; i < (int)IM_ARRAYSIZE(TD_EASINGS); ++i) {
                    bool sel = (i == easing_idx);
                    if (ImGui::Selectable(TD_EASINGS[i].name, sel)) {
                        easing_idx = i;
                        easeFn = TD_EASINGS[i].fn;
                    }
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            TD_Help("Different curves for the normalized time t in [0,1].");
        }

        // CHANNELS
        ImGui::Separator();

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Position X");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##ch_pos", &ch_pos); ImGui::SameLine();
        ImGui::DragFloatRange2("##pos_mm", &pos_min, &pos_max, 1.0f, -400.0f, 400.0f, "min=%.0f", "max=%.0f");
        TD_Help("Horizontal offset applied to the tile.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Size (W,H)");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##ch_size", &ch_size); ImGui::SameLine();
        ImGui::DragFloat2("##size_min", &size_min.x, 1.0f, 20.0f, 600.0f, "min=(%.0f,%.0f)");
        ImGui::DragFloat2("##size_max", &size_max.x, 1.0f, 20.0f, 600.0f, "max=(%.0f,%.0f)");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Rounding");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##ch_round", &ch_round); ImGui::SameLine();
        ImGui::DragFloatRange2("##round_mm", &r_min, &r_max, 0.5f, 0.0f, 64.0f, "min=%.1f", "max=%.1f");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Background");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##ch_bg", &ch_bg); ImGui::SameLine();
        ImGui::ColorEdit4("##bg_min", &bg_min.x, ImGuiColorEditFlags_NoInputs); ImGui::SameLine();
        ImGui::ColorEdit4("##bg_max", &bg_max.x, ImGuiColorEditFlags_NoInputs);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Text Alpha");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##ch_text", &ch_text); ImGui::SameLine();
        ImGui::DragFloatRange2("##a_mm", &a_min, &a_max, 0.01f, 0.0f, 1.0f, "min=%.2f", "max=%.2f");

        // RESET
        ImGui::Separator();
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Reset");
        ImGui::TableSetColumnIndex(1);
        if (ImGui::SmallButton("Reset to defaults"))
        {
            drive_mode = 0;
            inside_manual = false;
            auto_loop = false; auto_pause = 0.0f;

            upDur = 0.20f; downDur = 0.18f;
            easing_idx = TD_IndexFromEasing(Easing::easeInOutCubic);
            easeFn = TD_EASINGS[easing_idx].fn;

            ch_pos = ch_size = ch_round = ch_bg = true; ch_text = false;
            pos_min = 0.0f; pos_max = 120.0f;
            size_min = ImVec2(160, 38); size_max = ImVec2(240, 56);
            r_min = 4.0f; r_max = 16.0f;
            bg_min = ImVec4(0.14f, 0.14f, 0.16f, 1.0f);
            bg_max = ImVec4(0.35f, 0.22f, 0.70f, 1.0f);
            a_min = 0.40f; a_max = 1.00f;
        }

        ImGui::EndTable();
    }

    // Footer: quick tips
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextDisabled("Tips:");
    ImGui::BulletText("Try different up/down durations for asymmetrical feel (fast in, slow out).");
    ImGui::BulletText("Enable Text Alpha to demo independent channels animating together.");
    ImGui::BulletText("Switch Drive to Auto and add a Pause to see ping-pong tweening.");
}


