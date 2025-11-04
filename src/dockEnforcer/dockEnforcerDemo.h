#pragma once
#include "imguiDockEnforcer.h"
#include "demo_module.h"

class DockEnforcerDemo : public DemoModule
{
public:
	DockEnforcerDemo() : DemoModule("Dock Enforcer", "Dock Enforcer Demo Panel") {}
protected:
	void DrawSelectedDemo();
	void OnPrePanel() override;
	void OnPreDraw() override;
	void DrawDemoPanel() override;

	bool enforce_docking = true;
	bool specify_dock_target = false;
	std::string target_dock_window = "";
	int current_dock_window = 0;
};

inline void DockEnforcerDemo::DrawSelectedDemo()
{
	ImGui::TextWrapped("This demo shows how to use the Dock Enforcer utility to force a window to stay docked.");
	ImGui::TextWrapped("Use the button below to open a demo panel to test this feature!");
}

inline void DockEnforcerDemo::OnPrePanel()
{
	if (enforce_docking)
	{
		if (specify_dock_target && !target_dock_window.empty())
			ImGui::DockEnforcerBeginLock(panel_name.c_str(), target_dock_window.c_str());
		else
			ImGui::DockEnforcerBeginLock(panel_name.c_str());
	}
}

inline void DockEnforcerDemo::OnPreDraw()
{
	if (enforce_docking)
		ImGui::DockEnforcerEndLock();
}

inline void DockEnforcerDemo::DrawDemoPanel()
{
	constexpr const char* help_text =
		"The panel will start out floating, and has to be docked to ensure proper behavior.\n\n"
		"This panel is being forced to stay docked.\n"
		"If you try to drag it out of its dock space, it will snap back in place.\n"
		"You can change this behavior using the checkboxes below.\n";
	ImGui::TextWrapped(help_text);
	ImGui::Separator();
	ImGui::Checkbox("Enforce Docking?", &enforce_docking);
	ImGui::Checkbox("Specify Target Dock Window?", &specify_dock_target);
	if (specify_dock_target)
	{
		auto dock_windows = DemoManager::GetDemoModules();
		std::vector<const char*> names;
		names.reserve(dock_windows.size());
		for (auto& w : dock_windows)
			if (w->panel_name != panel_name && w->popout_open)  // don't include ourself
				names.push_back(w->panel_name.c_str());

		if (names.empty())
		{
			ImGui::TextDisabled("No other panels available.");
			ImGui::TextDisabled("Try opening other demo panels.");
		}
		else
		{
			if (current_dock_window >= (int)names.size())
				current_dock_window = 0;

			ImGui::Combo("Target Dock Window", &current_dock_window,
				names.data(), (int)names.size());
			
			target_dock_window = names[current_dock_window];
		}
	}
	else if (target_dock_window != "")
	{
		target_dock_window = "";
		current_dock_window = 0;
	}
}

