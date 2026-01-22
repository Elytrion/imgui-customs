#pragma once
#include "imguiCustomTooltip.h"
#include "demo_module.h"

class CustomTooltipDemo : public DemoModule
{
public:
	CustomTooltipDemo() : DemoModule("Custom Tooltip", "Custom Tooltip Demo Panel"){}
protected:
	void DrawSelectedDemo();
	void OnPrePanel() override;
	void DrawDemoPanel() override;
};

inline void CustomTooltipDemo::DrawSelectedDemo()
{
	ImGui::Spacing();
	ImGui::Text("Hover over me to see a custom tooltip.");
	ImGui::DrawCustomTooltip("This is a custom tooltip.");
	ImGui::Spacing();
	ImGui::Text("Hover (Top pivot tooltip)");
	ImGui::DrawCustomTooltip(
		"This tooltip is anchored to the top of the item.",
		ImVec2{ 0.5f, 0.0f }, // pivot
		ImVec2{ 0.0f, -5.0f }   // offset
	);
	ImGui::Spacing();
	ImGui::Text("Hover (Bottom Right pivot tooltip)");
	ImGui::DrawCustomTooltip(
		"This tooltip is anchored to the bottom-right of the item.",
		ImVec2{ 1.0f, 1.0f }, // pivot
		ImVec2{ 5.0f, 5.0f }   // offset
	);
}

inline void CustomTooltipDemo::OnPrePanel()
{
	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Appearing);
}

inline void CustomTooltipDemo::DrawDemoPanel()
{
	DrawPlaceholderText();
}


