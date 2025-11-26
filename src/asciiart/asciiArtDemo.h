#pragma once
#include "asciiCatAnim.h"
#include "imguiAnimatedASCIIArt.h"
#include "demo_module.h"

class ASCIIArtDemo : public DemoModule
{
	bool initialized = false;
	ImGui::AsciiAnimation mCatTailAnim;
	bool mCatAnimLoaded = false;
public:
	ASCIIArtDemo() : DemoModule("ASCII Art", "ASCII Art Demo Panel"){}

protected:
	void DrawSelectedDemo();
	void OnPrePanel() override;
	void DrawDemoPanel() override;
};

inline void ASCIIArtDemo::DrawSelectedDemo()
{
	if (!initialized)
	{
		// Load cat tail animation
		mCatAnimLoaded = ImGui::LoadAsciiAnimationFromMemory(ASCIIAnimations::kCatTailAnim, mCatTailAnim);
		mCatTailAnim.frame_duration = 0.2f;
		initialized = true;
	}

	ImGui::SeparatorText("ASCII Art Demo");
	ImGui::TextWrapped("This demo showcases some fun functions for drawing premade ASCII artwork using text.");
	ImGui::Spacing();

	ImGui::DrawASCIICatPeep();
	ImGui::DrawASCIIBunnyStare();

	if (mCatAnimLoaded)
		ImGui::DrawAsciiAnimation(mCatTailAnim);
	else
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Failed to load cat tail animation.");
}

inline void ASCIIArtDemo::OnPrePanel()
{
	
}

inline void ASCIIArtDemo::DrawDemoPanel()
{
	DrawPlaceholderText();
}


