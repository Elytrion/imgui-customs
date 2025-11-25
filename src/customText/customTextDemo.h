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

	ImGui::Spacing();
	ImGui::SeparatorText("Custom Text Formats");
	ImGui::Spacing();

	ImGui::TextLimitedF("This is some formatted text that is limited by the window space. "
		"Text that exceeds the limit is cut off and replaced with an ellipsis (...). "
		"The entire text will be shown in a tooltip when you hover over the text (toggleable). "
		"The tooltip should wrap around for you to see the entire text, with it's maximum width limited to the window width as well."
		"To prove formatting works, here is the display of current FPS: %.2f",
		ImGui::GetIO().Framerate);

	ImGui::Spacing();

	ImGui::TextWrappedLimited(
		"This is an example of wrapped text that is also limited in height. "
		"Text will first wrap, but if it will exceed a given height limit it will be cut off and replaced with an ellipsis (...). "
		"The entire text will be shown in a tooltip when you hover over the text (toggleable). "
		"This is useful for displaying potentially long paragraphs of text that does not consume too much UI space if the window size is changed."
		"Formatting is currently not available for this text type, and it may incur some slight performance penalties due to binary searching through the text provided each frame.",
		-1.0f,    // max width (use content region)
		-1.0f,    // max height in pixels (< 0 -> use max lines)
		3         // max lines (used if max height not specified)
	);
}

inline void CustomTextDemo::OnPrePanel()
{
	
}

inline void CustomTextDemo::DrawDemoPanel()
{
	DrawPlaceholderText();
}

