#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <functional>

// based on spinner from
// https://github.com/ocornut/imgui/issues/1901#issuecomment-552185000
namespace spinnerdetails
{
	// cubic Bezier (0,0) – (0.4,0) – (0.2,1) – (1,1) : Fast In, Slow Out
	constexpr inline float ease_bezier(float t) { return t * t * (3.0f - 2.0f * t); }

	//  Linear map with clamp to [0, 1]
	constexpr inline float interval(float t, float a, float b)
	{
		return (t <= a) ? 0.0f : (t >= b) ? 1.0f : (t - a) / (b - a);
	}

	// Saw-tooth that repeats N times over [0,1]
	template<int N>
	constexpr inline float saw(float t) { return std::fmod(t * N, 1.0f); }
}

namespace ImGui
{
    struct SpinnerConfig
    {
        // Sizes & colors
        float radius{ 12.0f };
        float indicator_thickness{ 3.0f };
        ImU32 indicator_color{ IM_COL32(255,255,255,255) };

        // Optional background track
        bool  show_track{ true };
        float track_thickness{ 1.0f };
        ImU32 track_color{ IM_COL32(255,255,255,64) };

        // Material-style animation params
        float min_arc{ 30.0f };   // degrees
        float max_arc{ 280.0f };  // degrees
        float period{ 5.0f };     // seconds for a full rotation
        int   detents{ 5 };       // only used by Material mode
        int   skip_detents{ 3 };  // only used by Material mode

        // New controls
        bool  constant_speed{ false }; // if true, constant angular velocity (no detents)
        bool  constant_arc{ false };   // if true, fixed indicator length (no grow/shrink)
        float arc_length_deg{ 90.0f }; // used when constant_arc is true

        std::function<void(ImVec2 spinner_center)> draw_fn = nullptr; // optional draw function (takes in the center of the spinner)
    };

    // New generic spinner
    inline bool Spinner(const char* label, const SpinnerConfig& cfg = {})
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        const ImVec2 pos = window->DC.CursorPos;
        const ImVec2 size = ImVec2(cfg.radius * 2.0f, (cfg.radius + style.FramePadding.y) * 2.0f);
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id)) return false;

        // Geometry helpers
        constexpr float start_angle = -IM_PI / 2.0f; // start at top
        const float min_arc_rad = cfg.min_arc * IM_PI / 180.0f;
        const float max_arc_rad = cfg.max_arc * IM_PI / 180.0f;
        const float fixed_arc_rad = cfg.arc_length_deg * IM_PI / 180.0f;

        // Common center
        const ImVec2 center = ImVec2(
            pos.x + cfg.radius,
            pos.y + cfg.radius + style.FramePadding.y
        );

        ImDrawList* dl = window->DrawList;

        // Optional background track
        if (cfg.show_track && cfg.track_thickness > 0.0f)
        {
            dl->PathArcTo(center, cfg.radius, 0.0f, 2.0f * IM_PI, 48);
            dl->PathStroke(cfg.track_color, false, cfg.track_thickness);
        }

        // Time base [0..1)
        const float t = std::fmod(g.Time, (cfg.period > 0.0f ? cfg.period : 1.0f)) /
            (cfg.period > 0.0f ? cfg.period : 1.0f);

        // Choose motion mode and compute [a_min, a_max]
        float a_min = 0.0f, a_max = 0.0f;

        if (!cfg.constant_speed && !cfg.constant_arc)
        {
            // -------- Material mode (your original) --------
            const int   num_detents = (cfg.detents > 0 ? cfg.detents : 5);
            const int   skip_detents = (cfg.skip_detents >= 0 ? cfg.skip_detents : 3);

            const float local = spinnerdetails::saw<5>(t); // default 5; detents in rotation math below
            const float head_u = spinnerdetails::ease_bezier(spinnerdetails::interval(local, 0.0f, 0.5f));
            const float tail_u = spinnerdetails::ease_bezier(spinnerdetails::interval(local, 0.5f, 1.0f));

            const float step = std::floor(t * num_detents);
            const float step_off = skip_detents * 2.0f * IM_PI / num_detents;
            const float rot_comp = std::fmod(4.0f * IM_PI - step_off - max_arc_rad, 2.0f * IM_PI);

            const float a_base = start_angle
                + tail_u * max_arc_rad
                + spinnerdetails::saw<5>(t) * rot_comp
                - step * step_off;

            a_min = a_base;
            a_max = a_base + (head_u - tail_u) * max_arc_rad + min_arc_rad;
        }
        else if (cfg.constant_speed && !cfg.constant_arc)
        {
            // -------- Constant RPM, variable arc --------
            // Keep the Material grow/shrink *shape* (ease between head/tail),
            // but rotate at a constant angular velocity with no detent jumps.
            const float head_u = spinnerdetails::ease_bezier(spinnerdetails::interval(t, 0.0f, 0.5f));
            const float tail_u = spinnerdetails::ease_bezier(spinnerdetails::interval(t, 0.5f, 1.0f));

            // Linear rotation: full 2 pi per period, minus max_arc so we don't wrap through
            const float rot_comp = (2.0f * IM_PI) - max_arc_rad;

            const float a_base = start_angle
                + tail_u * max_arc_rad   // growth start
                + t * rot_comp;          // linear rotation component

            a_min = a_base;
            a_max = a_base + (head_u - tail_u) * max_arc_rad + min_arc_rad;
        }
        else // (cfg.constant_arc) with either speed mode, but speed handled here linearly (most expected)
        {
            // -------- Constant RPM, fixed arc length --------
            const float omega = 2.0f * IM_PI;             // radians per period
            const float angle = start_angle + omega * t;  // pure linear rotation

            a_min = angle;
            a_max = angle + fixed_arc_rad;
        }

        dl->PathArcTo(center, cfg.radius, a_min, a_max, 30);
        dl->PathStroke(cfg.indicator_color, false, cfg.indicator_thickness);

        if (cfg.draw_fn)
            cfg.draw_fn(center);

        return true;
    }
}
