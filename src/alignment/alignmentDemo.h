#pragma once
#include "imguiAlignment.h"
#include "demo_module.h"

class AlignmentDemo : public DemoModule
{
public:
	AlignmentDemo() : DemoModule("Alignment", "Alignment Demo Panel"){}
protected:
	void DrawSelectedDemo();
	void OnPrePanel() override;
	void DrawDemoPanel() override;
};

inline void AlignmentDemo::DrawSelectedDemo()
{
	ImGui::Spacing();
	ImGui::TextWrapped("This demo shows how to use the custom alignment helpers. Stretch this window to see the alignment hold at different widths.");

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Button("Left Aligned");
	ImGui::SameLine();
	ImGui::CenterAlignedGroup("CenterAlignedButtonDemo", []()
		{
			if (ImGui::Button("Center Aligned"))
			{
				// Button action
			}
		});
	ImGui::SameLine();
	ImGui::RightAlignedGroup("RightAlignedButtonDemo", []()
		{
			if (ImGui::Button("Right Aligned"))
			{
				// Button action
			}
		});

	ImGui::Spacing();
}

inline void AlignmentDemo::OnPrePanel()
{
	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Appearing);
}

inline void AlignmentDemo::DrawDemoPanel()
{
	ImVec2 windowSize = ImGui::GetWindowSize();

	if (ImGui::BeginChild("alignment_demo_child", ImVec2(0, windowSize.y * 0.7), ImGuiChildFlags_Borders))
	{

		ImGui::AlignmentGroup("TopLeftGroup", AlignX::Left, AlignY::Top, [&]() { ImGui::Button("o", ImVec2(50, 50)); });
		ImGui::SameLine();
		ImGui::AlignmentGroup("TopCenterGroup", AlignX::Center, AlignY::Top, [&]() { ImGui::Button("o", ImVec2(50, 50)); });
		ImGui::SameLine();
		ImGui::AlignmentGroup("TopRightGroup", AlignX::Right, AlignY::Top, [&]() { ImGui::Button("o", ImVec2(50, 50)); });

		ImGui::Spacing();

		ImGui::AlignmentGroup("MiddleLeftGroup", AlignX::Left, AlignY::Middle, [&]() { ImGui::Button("o", ImVec2(50, 50)); });
		ImGui::SameLine();
		ImGui::AlignmentGroup("MiddleCenterGroup", AlignX::Center, AlignY::Middle, [&]() { ImGui::Button("o", ImVec2(50, 50)); });
		ImGui::SameLine();
		ImGui::AlignmentGroup("MiddleRightGroup", AlignX::Right, AlignY::Middle, [&]() { ImGui::Button("o", ImVec2(50, 50)); });

		ImGui::Spacing();

		ImGui::AlignmentGroup("BottomLeftGroup", AlignX::Left, AlignY::Bottom, [&]() { ImGui::Button("o", ImVec2(50, 50)); });
		ImGui::SameLine();
		ImGui::AlignmentGroup("BottomCenterGroup", AlignX::Center, AlignY::Bottom, [&]() { ImGui::Button("o", ImVec2(50, 50)); });
		ImGui::SameLine();
		ImGui::AlignmentGroup("BottomRightGroup", AlignX::Right, AlignY::Bottom, [&]() { ImGui::Button("o", ImVec2(50, 50)); });


		ImGui::EndChild();
	}

	ImGui::Spacing();

	ImGui::TextWrapped("The three alignment helpers (CenterAlignedGroup, RightAlignedGroup, and AlignmentGroup) use a two-pass system to measure the size of their contents and then position them accordingly. They use a simple caching mechanism to avoid re-measuring every frame.");
	ImGui::TextWrapped("The AlignmentGroup function is the most flexible, allowing you to specify both horizontal and vertical alignment within the window's content region. The CenterAlignedGroup and RightAlignedGroup are convenience wrappers for common use cases.");
	ImGui::TextWrapped("Feel free to resize the window to see how the alignment adapts dynamically.");


}


