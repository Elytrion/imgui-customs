#include <imgui.h>
#include <imgui_internal.h>
#include <vector>

namespace ImGui
{
	struct MultiToggleConfig
	{
		ImVec2  size{ 0,0 };     // (0,0) -> auto height, width = content avail
		float   rounding = -1.f;    // -1 = half height
		float   plate_pad = 6.f;     // inner padding around plate
		float   gap = 8.f;     // gap between unequal sections
		bool    equal_sections = true;    // true: even split; false: size by measured text
		float   anim_speed = 0.12f;   // seconds across full lerp
		float (*easing)(float) = nullptr; // optional easing, e.g. Easing::easeInOutCubic

		// Colors
        ImU32   col_track       = IM_COL32(125, 125, 125, 255);
        ImU32   col_track_hover = IM_COL32(100, 100, 100, 255);
        ImU32   col_plate       = IM_COL32(60, 120, 255, 230);
        ImU32   col_plate_hover = IM_COL32(60, 120, 255, 255);
        ImU32   col_text        = IM_COL32(10, 10, 10, 255);
        ImU32   col_text_active = IM_COL32(255, 255, 255, 255);
	};

	bool MultiToggle(
		const char* id, int* current,
		const std::vector<const char*>& labels,
		const MultiToggleConfig& cfg = {})
	{
        IM_ASSERT(current && !labels.empty() && labels.size() > 1);
        const int n = (int)labels.size();
        *current = ImClamp(*current, 0, n - 1);

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiStyle& style = ImGui::GetStyle();

        // ---------- Measure & layout ----------
        const float frameH = ImGui::GetFrameHeight();
        float h = cfg.size.y > 0.f ? cfg.size.y : frameH * 1.35f;
        float r = cfg.rounding >= 0.f ? cfg.rounding : h * 0.5f;

        // Measure text widths
        std::vector<float> ideal; ideal.reserve(n);
        float sumIdeal = 0.f;
        for (const char* s : labels) {
            float wtxt = ImGui::CalcTextSize(s).x + cfg.plate_pad * 2.f;
            if (wtxt < h) wtxt = h; // keep minimum comfortable slot
            ideal.push_back(wtxt);
            sumIdeal += wtxt;
        }

        // Track width
        float w = cfg.size.x > 0.f ? cfg.size.x : ImMax(sumIdeal + (cfg.equal_sections ? 0.f : cfg.gap * (n - 1)),
            ImGui::GetContentRegionAvail().x);

        // Build section rects
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImRect track_r(p, ImVec2(p.x + w, p.y + h));

        std::vector<ImRect> sections; sections.reserve(n);
        sections.resize(n);

        if (cfg.equal_sections) {
            float secW = w / n;
            for (int i = 0; i < n; ++i) {
                ImVec2 a(track_r.Min.x + secW * i, track_r.Min.y);
                ImVec2 b(a.x + secW, track_r.Max.y);
                sections[i] = ImRect(a, b);
            }
        }
        else {
            // Scale ideal widths to fit 'w' with gaps
            float totalGap = cfg.gap * (n - 1);
            float scale = (w - totalGap) / ImMax(1e-6f, sumIdeal);
            float x = track_r.Min.x;
            for (int i = 0; i < n; ++i) {
                float sw = ideal[i] * scale;
                sections[i] = ImRect(ImVec2(x, track_r.Min.y), ImVec2(x + sw, track_r.Max.y));
                x += sw + ((i < n - 1) ? cfg.gap : 0.f);
            }
        }

        // Helper to shrink plate inside a section
        auto shrink = [&](const ImRect& rr)
            {
                ImRect s = rr; s.Expand(ImVec2(-cfg.plate_pad, -cfg.plate_pad * 0.75f)); return s;
            };

        // ---------- Interaction ----------
        ImGui::InvisibleButton(id, track_r.GetSize());
        const bool hovered = ImGui::IsItemHovered();
        const bool pressed = ImGui::IsItemClicked();

        // Per-ID animation state (StateStorage), inspired by your Tween helper.
        // We store: fromIndex, toIndex, startTime, isAnimating.
        ImGuiStorage* st = window->DC.StateStorage;
        const ImGuiID wid = window->GetID(id);
        const ImGuiID kFrom = wid ^ 0x5101;  // int
        const ImGuiID kTo = wid ^ 0x5103;  // int
        const ImGuiID kStart = wid ^ 0x5107;  // float
        const ImGuiID kRun = wid ^ 0x510B;  // bool

        bool changed = false;

        if (pressed)
        {
            float mx = ImGui::GetIO().MousePos.x;
            // Find which section we clicked
            int desired = *current;
            for (int i = 0; i < n; ++i) {
                if (mx >= sections[i].Min.x && mx < sections[i].Max.x) { desired = i; break; }
            }

            if (desired != *current) {
                // Start a fresh animation only when index actually changes
                st->SetInt(kFrom, *current);
                st->SetInt(kTo, desired);
                st->SetFloat(kStart, (float)ImGui::GetTime());
                st->SetBool(kRun, true);
                *current = desired;
                changed = true;
            }
            // else: same index -> no animation restart
        }

        // ---------- Animate plate position ----------
        // Compute normalized playback 0..1 from our own timer (not LastActiveId).
        float t01 = 1.0f;
        bool  run = st->GetBool(kRun, false);
        int   to = st->GetInt(kTo, *current);
        int   from = st->GetInt(kFrom, *current);
        float stt = st->GetFloat(kStart, (float)ImGui::GetTime());

        if (run) {
            float norm = ImSaturate(((float)ImGui::GetTime() - stt) / ImMax(0.001f, cfg.anim_speed));
            t01 = cfg.easing ? cfg.easing(norm) : norm;
            if (norm >= 1.0f) {
                t01 = 1.0f;
                st->SetBool(kRun, false);
                from = to; // snap to target
            }
        }
        else {
            from = to = *current;
            t01 = 1.0f;
        }

        // Plate rect: lerp between shrunken section rects of from->to
        ImRect plateFrom = shrink(sections[from]);
        ImRect plateTo = shrink(sections[to]);
        ImRect plate;
        plate.Min = ImLerp(plateFrom.Min, plateTo.Min, t01);
        plate.Max = ImLerp(plateFrom.Max, plateTo.Max, t01);

        // ---------- Draw ----------
        ImU32 colTrack = hovered ? cfg.col_track_hover : cfg.col_track;
        ImU32 colPlate = hovered ? cfg.col_plate_hover : cfg.col_plate;

        ImDrawListSplitter split;
        split.Split(dl, 2);

        // Channel 0: track + plate (behind)
        split.SetCurrentChannel(dl, 0);
        dl->AddRectFilled(track_r.Min, track_r.Max, colTrack, r);
        dl->AddRectFilled(plate.Min, plate.Max, colPlate, r * 0.75f);

        // Channel 1: text (foreground)
        split.SetCurrentChannel(dl, 1);
        for (int i = 0; i < n; ++i) {
            ImRect sec = sections[i];
            const char* txt = labels[i];
            ImVec2 ts = ImGui::CalcTextSize(txt);
            ImVec2 pos{ sec.Min.x + (sec.GetWidth() - ts.x) * 0.5f,
                        sec.Min.y + (sec.GetHeight() - ts.y) * 0.5f };

            // Active text color if this index is the target (where plate lands)
            bool active = (i == to);
            dl->AddText(pos, active ? cfg.col_text_active : cfg.col_text, txt);
        }

        split.Merge(dl);
        ImGui::Dummy(track_r.GetSize()); // advance layout

        return changed;
	}
}
