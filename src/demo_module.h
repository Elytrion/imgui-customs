#pragma once
#include <string>

class DemoModule
{
public:
	DemoModule( const std::string& selectorName, const std::string& panelName) :
		selector_name(selectorName), panel_name(panelName) {}

	void DrawSelector();
	void DrawPopoutPanel();

	std::string selector_name = "Demo Module";
	std::string panel_name = "Demo Panel";
	bool popout_open = false;
	int panel_flags{ 0 };
protected:
	void virtual DrawSelectedDemo() = 0;
	void virtual OnPrePanel() {};
	void virtual OnPreDraw() {};
	void virtual DrawDemoPanel() = 0;
	void virtual OnPostDraw() {};
	void virtual OnPostPanel() {};
};

inline void DemoModule::DrawSelector()
{
	if (ImGui::CollapsingHeader(selector_name.c_str()))
	{
		DrawSelectedDemo();

		ImGui::Separator();
		if (ImGui::Button(("Open Demo Panel##" + selector_name).c_str()))
			popout_open = true;
	}
}

inline void DemoModule::DrawPopoutPanel()
{
	if (!popout_open)
		return;
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	OnPrePanel();
	bool drawn = ImGui::Begin(panel_name.c_str(), &popout_open, panel_flags);
	OnPreDraw();
	if (drawn)
		DrawDemoPanel();
	OnPostDraw();
	ImGui::Separator();
	if (ImGui::Button(("Close Panel##" + panel_name).c_str()))
		popout_open = false;
	ImGui::End();
	OnPostPanel();
}