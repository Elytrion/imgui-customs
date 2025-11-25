#pragma once
#include "imguiASCIIArt.h"
#include "demo_module.h"

class ASCIIArtDemo : public DemoModule
{
public:
	ASCIIArtDemo() : DemoModule("ASCII Art", "ASCII Art Demo Panel") {}
protected:
	void DrawSelectedDemo();
	void OnPrePanel() override;
	void DrawDemoPanel() override;
};

inline void ASCIIArtDemo::DrawSelectedDemo()
{
	ImGui::SeparatorText("ASCII Art Demo");
	ImGui::TextWrapped("This demo showcases some fun functions for drawing premade ASCII artwork using text.");
	ImGui::Spacing();

	ImGui::DrawASCIICatPeep();
	ImGui::DrawASCIIBunnyStare();
}

inline void ASCIIArtDemo::OnPrePanel()
{
	
}

inline void ASCIIArtDemo::DrawDemoPanel()
{
	DrawPlaceholderText();
}


