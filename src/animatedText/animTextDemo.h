#pragma once
#include "imguiAnimText.h"
#include "demo_module.h"

class AnimatedTextDemo : public DemoModule
{
public:
	AnimatedTextDemo() : DemoModule("Animated Text", "Animated Text Demo Panel") {}
protected:
	void DrawSelectedDemo();
	void OnPrePanel() override;
	void DrawDemoPanel() override;
};

inline void AnimatedTextDemo::DrawSelectedDemo()
{
	ImGui::SeparatorText("Animated Text Demo");
	ImGui::TextWrapped("This demo showcases a few examples of animated text effects."
	"Inspired by:");
	ImGui::TextLink("https://github.com/ocornut/imgui/issues/1286");
	ImGui::Spacing();
	ImGui::Separator();
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
}

inline void AnimatedTextDemo::OnPrePanel()
{
	
}

inline void AnimatedTextDemo::DrawDemoPanel()
{
	DrawPlaceholderText();
}

