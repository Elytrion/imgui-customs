#pragma once
#include <string>

class DemoModule
{
public:
	DemoModule( const std::string& selectorName, const std::string& panelName) :
		selector_name(selectorName), panel_name(panelName) {}

	void DrawSelector();
	void DrawPopoutPanel();
	void virtual BackgroundUpdate() {}; // Called once per frame, regardless of panel open state

	std::string selector_name = "Demo Module";
	std::string panel_name = "Demo Panel";
	bool popout_open = false;
	bool has_popout = true; // Set to false to disable popout demo panel functionality
	int panel_flags{ 0 };
protected:
	void virtual DrawSelectedDemo() = 0;
	void virtual OnPrePanel() {};
	void virtual OnPreDraw() {};
	void virtual DrawDemoPanel() {};
	void virtual OnPostDraw() {};
	void virtual OnPostPanel() {};
};

inline void DemoModule::DrawSelector()
{
	if (ImGui::CollapsingHeader(selector_name.c_str()))
	{
		DrawSelectedDemo();

		if (!has_popout)
			return;

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

static void DrawPlaceholderText()
{
	float t = (float)ImGui::GetTime();
	float hue = fmodf(t * 0.4f, 1.0f);
	ImVec4 col = ImColor::HSV(hue, 1.0f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, col);
	ImGui::Text("Placeholder Text");
	ImGui::PopStyleColor();
}

static void DrawHelpTooltip(const char* tooltip)
{
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 42.0f);
		ImGui::TextUnformatted(tooltip);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

static bool DrawEditColorU32(const char* label, ImU32* c)
{
	ImVec4 v = ImGui::ColorConvertU32ToFloat4(*c);
	bool changed = ImGui::ColorEdit4(label, &v.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
	if (changed) *c = ImGui::GetColorU32(v);
	return changed;
};