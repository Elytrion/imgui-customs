#pragma once
#include "imguiAnimText.h"
#include "imguiTextFormats.h"
#include "demo_module.h"

class CustomTextDemo : public DemoModule
{
public:
	CustomTextDemo() : DemoModule("Custom Text", "Custom Text Demo Panel") {}
protected:
	void DrawSelectedDemo();
	void OnPrePanel() override;
	void DrawDemoPanel() override;
};

inline void CustomTextDemo::DrawSelectedDemo()
{
	ImGui::SeparatorText("Custom Text Demo");
	ImGui::TextWrapped("This demo showcases a few examples of custom text effects not available by default in ImGUI."
	"Inspired by:");
	ImGui::TextLink("https://github.com/ocornut/imgui/issues/1286");
	ImGui::Spacing();
	ImGui::SeparatorText("Animated Text Effects");
	ImGui::Spacing();

	ImGui::TextWobble("This is an example of animated wobbling text!");

	ImGui::TextShaky("This is an example of animated shaky text!");

	const ImU32 col_red = IM_COL32(255, 50, 0, 255);
	const ImU32 col_blue = IM_COL32(0, 50, 255, 255);
	ImU32 colours[] = { col_red , col_blue };
	ImGui::TextGradient("This is an example of gradient colored text!", colours, 2);
	ImGui::TextGradientAnimated("This is an example of animated gradient colored text!", colours, 2);
	const ImU32 col_green = IM_COL32(0, 255, 50, 255);
	const ImU32 col_yellow = IM_COL32(255, 255, 0, 255);
	ImU32 colours2[] = { col_red , col_green, col_blue, col_yellow };
	ImGui::TextGradientAnimated("This is an example of animated gradient colored text with multiple stops!", colours2, 4, true, 0.2f, 0.5f);

    ImGui::TextWithAnimatedDots("This is some text with animated trailing dots");

	ImGui::TextMarquee("marquee1", "This is an example of a text marquee scrolling from right to left!");

	ImGui::Spacing();
	ImGui::SeparatorText("Custom Text Formats");
	ImGui::Spacing();

	ImGui::Spacing();

	ImGui::TextLimitedF("This is some formatted text that is limited by the window space. "
		"Text that exceeds the limit is cut off and replaced with an ellipsis (...). "
		"The entire text will be shown in a tooltip when you hover over the text (toggleable). "
		"The tooltip should wrap around for you to see the entire text, with it's maximum width limited to the window width as well."
		"To prove formatting works, here is the display of current FPS: %.2f",
		ImGui::GetIO().Framerate);

	ImGui::Spacing();

	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 200, 255, 255)); // added color for visibility
	ImGui::TextWrappedLimitedF(
		"STABLE_TEXT_ID_HERE", // must be provided for internal caching to work properly and must be stable and unique per text instance
		"This is an example of formatted wrapped text that is also limited in height. "
		"Text will first wrap, but if it will exceed a given height limit it will be cut off and replaced with an ellipsis (...). "
		"The entire text will be shown in a tooltip when you hover over the text (toggleable). "
		"This is useful for displaying potentially long paragraphs of text that does not consume too much UI space if the window size is changed. "
        "This text element may incur some slight performance penalties when recalculating text fitting for long paragraphs (> 300 words). "
		"To prove formatting works, here is the display of current FPS: %.2f",
        ImGui::GetIO().Framerate
	);
	ImGui::PopStyleColor();
}

inline void CustomTextDemo::OnPrePanel()
{
	
}

inline void CustomTextDemo::DrawDemoPanel()
{
    // --- persistent UI state for the demo ---
    static char text_buf[1024] =
        "This is some custom text! Try changing the type and settings to see different effects.";
    static int  text_type = 0; // 0=Plain, 1=Wobble, 2=Shaky, 3=Gradient, 4=Limited, 5=WrappedLimited

    // Wobble params
    static float wobble_amp = 3.0f;
    static float wobble_freq = 1.5f;
    static float wobble_speed = 1.0f;

    // Shaky params
    static float shaky_spread_x = 1.5f;
    static float shaky_spread_y = 3.0f;
    static float shaky_speed = 12.0f;
    static float shaky_chaos = 1.0f;
    static float shaky_roughness = 0.9f;

    // Gradient params
    static int grad_stopCount = 2;
	static constexpr int grad_maxStops = 16;
    static std::vector<ImU32> grad_stops = {
        IM_COL32(255, 80, 80, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(0, 130, 255, 255),
        IM_COL32(100, 100, 100, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
        IM_COL32(255, 255, 255, 255),
    };
    static ImU32 grad_inputStops[grad_maxStops];
    static bool  grad_animated = true;
    static bool  grad_pingpong = true;
    static float grad_speed = 1.0f;

    // TextLimited params
    static bool  tl_cut_left = false;
    static float tl_max_width = -1.0f;   // -1 = content region
    static float tl_padding_x = 0.0f;
    static int   tl_tooltip_mode = 0;     // 0 = full, 1 = cutoff, 2 = none

    // TextWrappedLimited params
    static float twl_max_width = -1.0f; // -1 = content region
    static float twl_max_height = -1.0f; // <0 = use max_lines
    static int   twl_max_lines = 3;
    static float twl_padding_x = 0.0f;
    static int   twl_tooltip_mode = 0;     // 0 = full, 1 = cutoff, 2 = none

    TextLimitedFlags tl_flags = TextLimitedFlags_TooltipShowAll;
    TextLimitedFlags twl_flags = TextLimitedFlags_TooltipShowAll;

    if (tl_tooltip_mode == 1)      tl_flags = TextLimitedFlags_TooltipShowCutoff;
    else if (tl_tooltip_mode == 2) tl_flags = TextLimitedFlags_NoTooltip;

    if (twl_tooltip_mode == 1)      twl_flags = TextLimitedFlags_TooltipShowCutoff;
    else if (twl_tooltip_mode == 2) twl_flags = TextLimitedFlags_NoTooltip;

    // --------------------------------------------------------------------
    // 1) PREVIEW
    // --------------------------------------------------------------------
    ImGui::SeparatorText("Preview");

    ImGui::BeginGroup();
    switch (text_type)
    {
    case 0: // Plain
        ImGui::TextUnformatted(text_buf);
        break;
    case 1: // Wobble
        ImGui::TextWobble(text_buf, wobble_amp, wobble_freq, wobble_speed);
        break;
    case 2: // Shaky
        ImGui::TextShaky(text_buf, shaky_spread_x, shaky_spread_y, shaky_speed, shaky_chaos, shaky_roughness);
        break;
    case 3: // Gradient / Gradient animated
    {
        for (int i = 0; i < grad_stopCount; ++i)
        {
            grad_inputStops[i] = grad_stops[i];
        }
        if (!grad_animated)
        {
            ImGui::TextGradient(text_buf, grad_inputStops, grad_stopCount);
        }
        else
        {
            ImGui::TextGradientAnimated(text_buf, grad_inputStops, grad_stopCount, grad_pingpong, grad_speed);
        }
        break;
    }
    case 4: // Single-line limited
        ImGui::TextLimited(text_buf, tl_cut_left, tl_max_width, tl_padding_x, tl_flags);
        break;
    case 5: // Wrapped + height-limited
        ImGui::TextWrappedLimited(
            "CustomTextDemo_WrappedPreview", // stable/unique ID for caching
            text_buf,
            twl_max_width,
            twl_max_height,
            twl_max_lines,
            twl_padding_x,
            twl_flags);
        break;
    }
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();

    // --------------------------------------------------------------------
    // 2) INPUT TEXT
    // --------------------------------------------------------------------
    ImGui::SeparatorText("Text Input");
    ImGui::InputTextMultiline("##CustomTextDemoInput", text_buf, IM_ARRAYSIZE(text_buf),
        ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 5.0f), ImGuiInputTextFlags_WordWrap);

    ImGui::Spacing();
    ImGui::Separator();

    // --------------------------------------------------------------------
    // 3) TEXT TYPE SELECTION
    // --------------------------------------------------------------------
    ImGui::SeparatorText("Text Type");
    const char* text_type_items[] = {
        "Plain",
        "Wobble (Animated)",
        "Shaky (Animated)",
        "Gradient / Gradient Animated",
        "Limited (Single-line, Ellipsis)",
        "Wrapped Limited (Multi-line, Height Clamp)"
    };
    ImGui::Combo("Type", &text_type, text_type_items, IM_ARRAYSIZE(text_type_items));

    ImGui::Spacing();
    ImGui::Separator();

    // --------------------------------------------------------------------
    // 4) PER-TYPE SETTINGS
    // --------------------------------------------------------------------
    ImGui::SeparatorText("Settings");

    switch (text_type)
    {
    case 1: // Wobble
        ImGui::TextDisabled("Wobble Settings");
        ImGui::SliderFloat("Amplitude", &wobble_amp, 0.0f, 20.0f, "%.1f px");
		DrawHelpTooltip("Wobble amplitude in pixels.");
        ImGui::SliderFloat("Frequency", &wobble_freq, 0.1f, 5.0f, "%.2f waves");
		DrawHelpTooltip("Number of complete wave cycles per text length.");
        ImGui::SliderFloat("Speed", &wobble_speed, 0.0f, 5.0f, "%.2f cycles/s");
		DrawHelpTooltip("Wobble speed in cycles per second.");
        break;

    case 2: // Shaky
        ImGui::TextDisabled("Shaky Settings");
        ImGui::SliderFloat("Spread X", &shaky_spread_x, 0.0f, 10.0f, "%.1f px");
		DrawHelpTooltip("Maximum horizontal displacement in pixels.");
        ImGui::SliderFloat("Spread Y", &shaky_spread_y, 0.0f, 20.0f, "%.1f px");
		DrawHelpTooltip("Maximum vertical displacement in pixels.");
        ImGui::SliderFloat("Speed", &shaky_speed, 0.0f, 30.0f, "%.1f");
		DrawHelpTooltip("Shakiness speed (higher = faster movement).");
        ImGui::SliderFloat("Chaos", &shaky_chaos, 0.0f, 3.0f, "%.2f");
		DrawHelpTooltip("Amount of randomness between characters (higher = more desync).");
        ImGui::SliderFloat("Roughness", &shaky_roughness, 0.0f, 1.0f, "%.2f");
		DrawHelpTooltip("Movement smoothness (0=sharp, 1=smooth).");
        break;

    case 3: // Gradient
        ImGui::TextDisabled("Gradient Settings");
        ImGui::SliderInt("Color Stops", &grad_stopCount, 1, grad_maxStops);
        for (int i = 0; i < grad_stopCount; i++)
        {
            char label[64];
            snprintf(label, sizeof(label), "Color %d", i);
            DrawEditColorU32(label, &grad_stops[i]);
        }
        ImGui::Checkbox("Animated", &grad_animated);
        if (grad_animated)
        {
            ImGui::Checkbox("Ping-Pong", &grad_pingpong);
            ImGui::SliderFloat("Speed", &grad_speed, 0.1f, 5.0f, "%.2f widths/s");
        }
        break;

    case 4: // TextLimited
    {
        ImGui::TextDisabled("Single-line Limited Settings");

        ImGui::Checkbox("Cut Left", &tl_cut_left);
        ImGui::SliderFloat("Max Width", &tl_max_width, -1.0f, 800.0f,
            (tl_max_width < 0.0f) ? "Content Region" : "%.0f px");
        ImGui::SliderFloat("Padding X", &tl_padding_x, 0.0f, 20.0f, "%.1f px");

        const char* tooltip_modes[] = { "Show full text", "Show only cutoff", "No tooltip" };
        ImGui::Combo("Tooltip Mode", &tl_tooltip_mode, tooltip_modes, IM_ARRAYSIZE(tooltip_modes));
        break;
    }

    case 5: // TextWrappedLimited
    {
        ImGui::TextDisabled("Wrapped Limited Settings");

        ImGui::SliderFloat("Max Width", &twl_max_width, -1.0f, 800.0f,
            (twl_max_width < 0.0f) ? "Content Region" : "%.0f px");
        ImGui::SliderFloat("Max Height", &twl_max_height, -1.0f, 400.0f,
            (twl_max_height < 0.0f) ? "Use Max Lines" : "%.0f px");
        ImGui::SliderInt("Max Lines", &twl_max_lines, 1, 15);
        ImGui::SliderFloat("Padding X", &twl_padding_x, 0.0f, 20.0f, "%.1f px");

        const char* tooltip_modes[] = { "Show full text", "Show only cutoff", "No tooltip" };
        ImGui::Combo("Tooltip Mode", &twl_tooltip_mode, tooltip_modes, IM_ARRAYSIZE(tooltip_modes));
        break;
    }

    default:
        ImGui::TextDisabled("No extra settings for this type.");
        break;
    }
}


