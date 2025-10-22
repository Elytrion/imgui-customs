#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "demo_module.h"

class HelpModule : public DemoModule
{
public:
	HelpModule() : DemoModule("Help", " ")
	{
		has_popout = false;
	}
protected:
	void OnPreSelectable() override;
	void DrawSelectedDemo() override;
	void OnPostSelectable() override;
};

void HelpModule::OnPreSelectable()
{
	ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 171, 120, 255));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 194, 133, 255));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 185, 130, 255));
}
void HelpModule::DrawSelectedDemo()
{
	ImGui::SeparatorText("USER GUIDE:");
	DrawPlaceholderText();
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 3.0f);
}
void HelpModule::OnPostSelectable()
{
	ImGui::PopStyleColor(3);
}