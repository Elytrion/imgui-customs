#pragma once
#include "imguiProgressBar.h"
#include "demo_module.h"

class ProgressBarDemo : public DemoModule
{
	float selection_progress_value = 0.0f;
	bool stop_selection_progress = false;
    ImGui::BufferingBarConfig cfg;  // user-tweakable config
    bool   manual = false;          // manual vs auto progress
    float  progress = 0.0f;         // current target progress [0..1]
    float  auto_rate_hz = 0.25f;    // cycles per second for auto mode (wraps 0..1)
    char   label_buf[64] = "buffer_bar_demo";
	bool   inProgress = true;    // show the dots
public:
	ProgressBarDemo() : DemoModule("Progress Bar", "Progress Bar Demo Panel") {}
protected:
	void DrawSelectedDemo();
    void OnPrePanel() override;
	void DrawDemoPanel() override;
};

void ProgressBarDemo::DrawSelectedDemo()
{
	ImGui::Text("Progress Bar Showcase");
	ImGui::TextWrapped(
		"This demo shows a simple configurable progress bar with basic lerp and animations. "
		"This widget was inspired by: ");
	ImGui::TextLink("https://github.com/ocornut/imgui/issues/1901#issuecomment-1951343738");
	ImGui::Separator();

	float t = (float)ImGui::GetTime();
	ImGui::BufferingBar(("Buffering Bar ##" + selector_name).c_str(), selection_progress_value, true);
	(ImGui::Checkbox(("Manual Progress ##" + selector_name).c_str(), &stop_selection_progress));
	if (stop_selection_progress) {
		ImGui::SliderFloat(("Progress ##" + selector_name).c_str(), &selection_progress_value, 0.0f, 1.0f, "%.3f");
	}
    else
    {
        const float ramp_dur = 5.0f; 
        const float hold_dur = 0.2f; 
        const float period = ramp_dur + hold_dur * 2;
        float phase = fmodf(t, period);

        if (phase < ramp_dur) {
            selection_progress_value = phase / ramp_dur;
        }
        else if (phase < ramp_dur + hold_dur) {
            selection_progress_value = 1.0f;
        }
        else {
            selection_progress_value = 0.0f;
        }
    }
}

void ProgressBarDemo::OnPrePanel()
{
	ImGui::SetNextWindowSize(ImVec2(500, 620), ImGuiCond_Appearing);
}

void ProgressBarDemo::DrawDemoPanel()
{
    // Reasonable defaults once
    static bool inited = false;
    if (!inited)
    {
        inited = true;
        cfg.size = ImVec2(0, 5);       // auto: width=content, height=FrameHeight
        cfg.rounding = 4.0f;

        cfg.num_circles = 3;
        cfg.circle_span = 0.30f;              // 30% of bar reserved for dots
        cfg.circle_period = 1.50f;              // seconds per dot loop (smaller = faster)

        cfg.smooth_progress = true;
        cfg.smooth_duration = 0.15f;

        cfg.snap_finish = true;               // snap when hitting 1.0
        cfg.snap_start = false;              // smooth back to 0 by default

        // 0 = pick from style each frame
        cfg.col_bar_bg = 0;
        cfg.col_bar_fg = 0;
        cfg.col_circles = 0;
    }

    ImGui::TextUnformatted("Buffering Bar Playground");
    ImGui::Separator();

    // =========================
    // Preview (bordered area)
    // =========================
    ImGui::BeginChild("progress_preview_child", ImVec2(0, 0),
        ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
    {
        // Drive target progress
        if (!manual)
        {
            float t = (float)ImGui::GetTime();
            progress = fmodf(t * auto_rate_hz, 1.0f); // wraps 0..1
        }

        // Live widget
        ImGui::TextUnformatted("Preview");
        ImGui::SameLine(220.0f);
        ImGui::SetNextItemWidth(-1.0f); // fill remaining width nicely
        ImGui::BufferingBar(label_buf, progress, inProgress, cfg);

        // Status line
        ImGui::Text("Target: %.3f | Smooth: %s | Duration: %.2fs",
            progress,
            (cfg.smooth_progress ? "On" : "Off"),
            cfg.smooth_duration);
        ImGui::Text("Dots: %d | Span: %.0f%% | Period: %.2fs | Label: \"%s\"",
            cfg.num_circles, cfg.circle_span * 100.0f, cfg.circle_period, label_buf);
    }
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::SeparatorText("Settings");

    // =========================
    // Settings table (Label | Value)
    // =========================
    if (ImGui::BeginTable("buffer_cfg_table", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 190.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        // ---------- Progress driving ----------
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Manual Progress");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##manual", &manual);
        DrawHelpTooltip("If off, progress auto-cycles 0..1 at the given rate.");

        if (manual)
        {
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Progress");
            ImGui::TableSetColumnIndex(1);
            ImGui::SliderFloat("##progress", &progress, 0.0f, 1.0f, "%.3f");
        }
        else
        {
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Auto Rate (Hz)");
            ImGui::TableSetColumnIndex(1);
            ImGui::DragFloat("##auto_rate", &auto_rate_hz, 0.01f, 0.01f, 10.0f, "%.2f");
            DrawHelpTooltip("Cycles per second (wraps 0..1).");
        }

		ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Animate Dots");
		ImGui::TableSetColumnIndex(1);
		ImGui::Checkbox("##inProgress", &inProgress);
		DrawHelpTooltip("Show the moving dots (right region).");

        // ---------- Geometry ----------
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Size (W,H)");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat2("##size", &cfg.size.x, 1.0f, 0.0f, 4096.0f, "%.1f");
        DrawHelpTooltip("(0,0) uses content width & FrameHeight().");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Track Rounding");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat("##rounding", &cfg.rounding, 0.2f, 0.0f, 64.0f, "%.1f");

        // ---------- Dots region ----------
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Dots: Count");
        ImGui::TableSetColumnIndex(1);
        ImGui::SliderInt("##num_circles", &cfg.num_circles, 0, 12);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Dots: Span");
        ImGui::TableSetColumnIndex(1);
        ImGui::SliderFloat("##circle_span", &cfg.circle_span, 0.0f, 0.5f, "%.2f");
        DrawHelpTooltip("Fraction of total width (0 = no dots, 0.5 = half bar).");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Dots: Period (s)");
        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat("##circle_period", &cfg.circle_period, 0.01f, 0.05f, 10.0f, "%.2f");
        DrawHelpTooltip("Seconds for one dot to traverse the dot region.");

        // ---------- Smoothing ----------
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Smooth Progress");
        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("##smooth", &cfg.smooth_progress);
        DrawHelpTooltip("When enabled, display chases target smoothly (stable for streaming inputs).");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Smooth Duration (s)");
        ImGui::TableSetColumnIndex(1);
        ImGui::BeginDisabled(!cfg.smooth_progress);
        ImGui::DragFloat("##smooth_dur", &cfg.smooth_duration, 0.01f, 0.01f, 2.0f, "%.2f");
        ImGui::EndDisabled();
        DrawHelpTooltip("Time constant: smaller = snappier, larger = floatier.");

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Snap on Finish / Reset");
        ImGui::TableSetColumnIndex(1);
        ImGui::BeginDisabled(!cfg.smooth_progress);
        ImGui::Checkbox("Finish##snap_finish", &cfg.snap_finish); ImGui::SameLine();
        ImGui::Checkbox("Reset##snap_start", &cfg.snap_start);
        ImGui::EndDisabled();
        DrawHelpTooltip("Snap instantly at 1.0 and/or 0.0 instead of smoothing.");

        // ---------- Colors ----------
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::SeparatorText("Colors");
        ImGui::TableSetColumnIndex(1);

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Bar Background");
        ImGui::TableSetColumnIndex(1);
        DrawEditColorU32("##bar_bg", &cfg.col_bar_bg); ImGui::SameLine();
        if (ImGui::SmallButton("Use Style##bg")) cfg.col_bar_bg = 0;

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Bar Progress");
        ImGui::TableSetColumnIndex(1);
        DrawEditColorU32("##bar_fg", &cfg.col_bar_fg); ImGui::SameLine();
        if (ImGui::SmallButton("Use Style##fg")) cfg.col_bar_fg = 0;

        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::Text("Dots");
        ImGui::TableSetColumnIndex(1);
        DrawEditColorU32("##dots", &cfg.col_circles); ImGui::SameLine();
        if (ImGui::SmallButton("Use Style##dots")) cfg.col_circles = 0;

        // ---------- Reset ----------
        ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Reset");
        ImGui::TableSetColumnIndex(1);
        if (ImGui::SmallButton("Reset to defaults"))
        {
            cfg = ImGui::BufferingBarConfig{};
            cfg.rounding = 4.0f;
            cfg.smooth_progress = true;
            cfg.smooth_duration = 0.15f;
            cfg.snap_finish = true;
            cfg.snap_start = false;
            progress = 0.0f;
            manual = false;
            auto_rate_hz = 0.25f;
            strcpy(label_buf, "buffer_bar_demo");
        }

        ImGui::EndTable();
    }
}


