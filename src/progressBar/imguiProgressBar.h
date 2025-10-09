#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include "easing.h"

namespace ImGui
{
    struct BufferingBarConfig
    {
        // Geometry
		ImVec2 size = ImVec2(0, 5.0f);      // (0,0) -> width = ContentRegionAvail.x, height = FrameHeight() [default height = 5, looks best]
        float  rounding = 0.0f;             // track rounding (0 = square)
        // Circles area
        int    num_circles = 3;             // how many moving dots
        float  circle_span = 0.20f;         // fraction of the total width reserved for circles [0..0.5], 0 = no circles
        float  circle_period = 1.50f;       // seconds for a dot to traverse the circles region (loop)
        // Progress animation
        bool   smooth_progress = true;      // true = lerp to target value over time
        float  smooth_duration = 0.1f;      // seconds to lerp from current->target (used if smooth_progress)
		bool   snap_finish{ true };			// (used in smoothing) if true, if progress set to 1.0, snap to 1.0 immediately [Makes completions more responsive]
		bool   snap_start{ true };		    // (used in smoothing) if true, if progress set to 0.0, snap to 0.0 immediately [Makes resets more responsive]
        // Colors (0 means "derive from style" each frame)
        ImU32  col_bar_bg = 0;              // background track (left region)
        ImU32  col_bar_fg = 0;              // progress fill (left region)
		ImU32  col_circles = 0;             // moving dots color (right region) [By default, same as col_bar_bg]
    };

    inline bool BufferingBar(const char* label, float value, bool inProgress, const BufferingBarConfig& cfg = {})
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
        ImU32 col_circles = cfg.col_circles ? cfg.col_circles : GetColorU32(ImGuiCol_FrameBg);

        // --- Progress value (optionally smoothed) ----------------------------
        float target = ImSaturate(value);
        float display = target; // default: immediate

        if (cfg.smooth_progress && cfg.smooth_duration > 0.0f)
        {
            ImGuiStorage* st = GetStateStorage();
            const ImGuiID kDisplay = id;
            const ImGuiID kLastTime = id + 10;

            const float kEps = 1e-5f;
            const bool snap_up = cfg.snap_finish && (target >= 1.0f - kEps);
            const bool snap_down = cfg.snap_start && (target <= 0.0f + kEps);

            if (snap_up || snap_down)
            {
                // Snap to the endpoint immediately
                display = target;
                st->SetFloat(kDisplay, display);
                st->SetFloat(kLastTime, (float)g.Time);
            }
            else
            {
                // Continuous chase integrator (stable when target moves every frame)
                float prev_display = st->GetFloat(kDisplay, target);
                float last_time = st->GetFloat(kLastTime, (float)g.Time);

                float now = (float)g.Time;
                float dt = ImMax(0.0f, now - last_time);

                // Convert "duration to feel mostly there" into an exponential smoothing factor
                float tau = ImMax(1e-6f, cfg.smooth_duration);
                float alpha = 1.0f - expf(-dt / tau);            // [0..1]
                float w = alpha;

                display = ImLerp(prev_display, target, w);

                st->SetFloat(kDisplay, display);
                st->SetFloat(kLastTime, now);
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
        if (clamped_span > 0.0f && cfg.num_circles > 0 && circle_width > 0.0f && inProgress)
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
