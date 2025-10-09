#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include "easing.h"

namespace ImGui
{
    struct BufferingBarConfig
    {
        // Geometry
        ImVec2 size = ImVec2(0, 0);   // (0,0) -> width = ContentRegionAvail.x, height = FrameHeight()
        float  rounding = 0.0f;           // track rounding (0 = square)
        // Circles area
        int    num_circles = 3;              // how many moving dots
        float  circle_span = 0.30f;          // fraction of the total width reserved for circles [0..0.5], 0 = no circles
        float  circle_period = 1.50f;          // seconds for a dot to traverse the circles region (loop)
        // Progress animation
        bool   smooth_progress = false;          // true = lerp to target value over time
        float  smooth_duration = 0.15f;          // seconds to lerp from current->target (used if smooth_progress)
        float (*easing)(float) = nullptr;        // easing for smoothing; nullptr = linear (t)
		bool snap_finish{ true };				 // (used in smoothing) if true, if progress set to 1.0, snap to 1.0 immediately
        // Colors (0 means "derive from style" each frame)
        ImU32  col_bar_bg = 0;              // background track (left region)
        ImU32  col_bar_fg = 0;              // progress fill (left region)
        ImU32  col_circles = 0;              // moving dots color (right region)
    };

    inline bool BufferingBarEx(const char* label, float value, const BufferingBarConfig& cfg = {})
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        // --- Geometry --------------------------------------------------------
        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = cfg.size;
        if (size.x <= 0.0f) size.x = GetContentRegionAvail().x;
        if (size.y <= 0.0f) size.y = GetFrameHeight();

        // Keep original layout behavior (shrink width by paddings, like your code)
        size.x -= style.FramePadding.x * 2.0f;

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(bb, id))
            return false;

        // Track split: left = bar area, right = circles area
        const float clamped_span = ImClamp(cfg.circle_span, 0.0f, 0.5f);
        const float circle_start = size.x * (1.0f - clamped_span);
        const float circle_end = size.x;
        const float circle_width = circle_end - circle_start;

        // Rounded corners for the filled rectangles
        const float track_rounding = cfg.rounding;

        // --- Colors (from style if not provided) -----------------------------
        ImU32 col_bar_bg = cfg.col_bar_bg ? cfg.col_bar_bg : GetColorU32(ImGuiCol_FrameBg);
        ImU32 col_bar_fg = cfg.col_bar_fg ? cfg.col_bar_fg : GetColorU32(ImGuiCol_PlotHistogram);
        ImU32 col_circles = cfg.col_circles ? cfg.col_circles : GetColorU32(ImGuiCol_TextDisabled);

        // --- Progress value (optionally smoothed) ----------------------------
        float target = ImSaturate(value);

        float display = target; // default: immediate
        if (cfg.smooth_progress && cfg.smooth_duration > 0.0f)
        {
            // Storage keys derived from id (unique per widget instance)
            ImGuiStorage* st = GetStateStorage();
            const ImGuiID kDisplay = id;         // current displayed value
            const ImGuiID kStartVal = id + 1;     // anim start value
            const ImGuiID kStartTime = id + 2;     // anim start time
            const ImGuiID kPrevTarget = id + 3;     // previously seen target

            float prev_target = st->GetFloat(kPrevTarget, -1.0f);
            float cur_display = st->GetFloat(kDisplay, target); // init to target on first frame

            // If target changed, (re)start a lerp from current display
            if (prev_target < 0.0f || fabsf(prev_target - target) > 1e-6f)
            {
                st->SetFloat(kStartVal, cur_display);
                st->SetFloat(kStartTime, (float)g.Time);
                st->SetFloat(kPrevTarget, target);
            }

            const float t = ((float)g.Time - st->GetFloat(kStartTime, (float)g.Time)) / cfg.smooth_duration;
            const float t01 = ImSaturate(t);
            const float eased = cfg.easing ? cfg.easing(t01) : t01;
            display = ImLerp(st->GetFloat(kStartVal, target), target, eased);

            // Snap to target when done
            if (t01 >= 1.0f)
            {
                display = target;
                st->SetFloat(kDisplay, display);
                st->SetFloat(kStartVal, display);
                st->SetFloat(kPrevTarget, target);
            }
            else
            {
                st->SetFloat(kDisplay, display);
            }
        }

        // --- Draw bar background & progress (left region) --------------------
        // Background for the left region (full left region)
        window->DrawList->AddRectFilled(
            bb.Min,
            ImVec2(pos.x + circle_start, bb.Max.y),
            col_bar_bg, track_rounding);

        // Progress fill (clamped to left region)
        const float prog_w = circle_start * ImSaturate(display);
        if (prog_w > 0.0f)
        {
            window->DrawList->AddRectFilled(
                bb.Min,
                ImVec2(pos.x + prog_w, bb.Max.y),
                col_bar_fg, track_rounding);
        }

        // --- Draw circles (right region) -------------------------------------
        if (clamped_span > 0.0f && cfg.num_circles > 0 && circle_width > 0.0f)
        {
            const float r = size.y * 0.5f;
            const float period = (cfg.circle_period > 0.0f) ? cfg.circle_period : 1e-6f;
            const float t = (float)g.Time;

            // The dot travels across [circle_start..circle_end] plus one radius of overshoot,
            // so it fully disappears at the right edge before wrapping.
            const float travel = circle_width + r;

            // Stagger phases evenly across the loop
            for (int i = 0; i < cfg.num_circles; ++i)
            {
                float shift = (period / ImMax(1, cfg.num_circles)) * i;
                float phase = fmodf(t + shift, period) / period;       // [0..1)
                float x = pos.x + circle_end - phase * travel;
                ImVec2 c = ImVec2(x, bb.Min.y + r);
                window->DrawList->AddCircleFilled(c, r, col_circles);
            }
        }

        return true;
    }
}
