#pragma once
#include "imguiMultiToggle.h"
#include "demo_module.h"
#include "easing.h"
#include <array>

class MultiToggleDemo : public DemoModule
{
public:
	MultiToggleDemo() : DemoModule("Multi-Toggle", "Multi-Toggle Demo Panel") {}
protected:
	void DrawSelectedDemo();
    void OnPrePanel() override;
	void DrawDemoPanel() override;
};

void MultiToggleDemo::DrawSelectedDemo()
{
	ImGui::TextWrapped("This is a demo of the MultiToggle widget, which allows selection among multiple options with smooth animations and customizable styles.");
	static int current = 0;
	bool changed = ImGui::MultiToggle("##multi", &current, { "Option 1", "Option 2", "Option 3", "Option 4" });
	ImGui::Text("Current selection: Option %d", current + 1);
}

void MultiToggleDemo::OnPrePanel()
{
	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);
}

void MultiToggleDemo::DrawDemoPanel()
{

}




