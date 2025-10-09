#pragma once
#include "imguiProgressBar.h"
#include "demo_module.h"

class ProgressBarDemo : public DemoModule
{
 
public:
	ProgressBarDemo() : DemoModule("Progress Bar", "Progress Bar Demo Panel") {}
protected:
	void DrawSelectedDemo();
    void OnPrePanel() override;
	void DrawDemoPanel() override;
};

void ProgressBarDemo::DrawSelectedDemo()
{
	DrawPlaceholderText();
}

void ProgressBarDemo::OnPrePanel()
{
	ImGui::SetNextWindowSize(ImVec2(500, 620), ImGuiCond_Appearing);
}

void ProgressBarDemo::DrawDemoPanel()
{
	DrawPlaceholderText();
}


