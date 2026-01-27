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
	ImGui::BulletText("Click on the title bars to open and collapse the module showcases.");
	ImGui::BulletText("Each module here showcases a custom collection of header-only ImGui widgets and helpers.");
	ImGui::BulletText("Each module (demo and headers) are contained in individual folders in this projects src folder.");
	ImGui::BulletText("Each module consists of a showcase and demo panel.");
	ImGui::Indent();
	ImGui::BulletText("The showcase is embedded in this main window and is a small demonstration of the module widgets.");
	ImGui::BulletText("The demo panel is a separate window that can be opened by clicking the 'Open Demo Panel' button.");
	ImGui::BulletText("The demo panel contains a more complete demonstration of the module widgets.");
	ImGui::BulletText("It typically includes customisation options and additional examples.");
	ImGui::BulletText("You can open multiple demo panels at once");
	ImGui::Unindent();
	ImGui::BulletText("Feel free to undock this main window to have more space for the demo panels.");
	ImGui::BulletText("Each module is independent and can be extracted and used in your own projects.");
	ImGui::BulletText("You can do this by simply including the relevant header files from the module folder.");
	ImGui::BulletText("No additional setup or configuration is required.");
	ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.0f);
}
void HelpModule::OnPostSelectable()
{
	ImGui::PopStyleColor(3);
}