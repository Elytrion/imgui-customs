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

}

inline void AnimatedTextDemo::OnPrePanel()
{
	
}

inline void AnimatedTextDemo::DrawDemoPanel()
{
	DrawPlaceholderText();
}

