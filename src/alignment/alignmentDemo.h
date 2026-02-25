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

	bool mRemovedWindowPadding = false;
	bool mModifyOffset = false;
	bool mWasModifyingOffset = false;
	bool mShowMultiElementAlignment = false;
	ImVec2 mOffsets[9];
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

	ImGui::HorizontalMidGroup row("horizontalRow");

	row.DrawWidget("##item1", [&]() { ImGui::Button("these buttons", ImVec2(0, 20.0f)); })
		.DrawWidget("##item2", [&]() { ImGui::Button("are all aligned by their height midpoints", ImVec2(0,40.0f)); })
		.DrawWidget("##item3", [&]() { ImGui::Button("in a horizontal row", ImVec2(0, 30.0f)); })
		.End();
}

inline void AlignmentDemo::OnPrePanel()
{
	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Appearing);
}

inline void AlignmentDemo::DrawDemoPanel()
{
	ImVec2 windowSize = ImGui::GetWindowSize();

	if (mRemovedWindowPadding)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	}
	if (ImGui::BeginChild("alignment_demo_child", ImVec2(0, windowSize.y * 0.5f), ImGuiChildFlags_Borders))
	{
		const ImVec2 btnSize = ImVec2(25.0f, 25.0f);
		ImGui::AlignmentGroup("TopLeftGroup", AlignX::Left, AlignY::Top, [&]() { ImGui::Button("o", btnSize); }, mOffsets[0]);
		ImGui::SameLine();
		ImGui::AlignmentGroup("TopCenterGroup", AlignX::Center, AlignY::Top, [&]() { ImGui::Button("o", btnSize); }, mOffsets[1]);
		ImGui::SameLine();
		ImGui::AlignmentGroup("TopRightGroup", AlignX::Right, AlignY::Top, [&]() { ImGui::Button("o", btnSize); }, mOffsets[2]);

		ImGui::Spacing();

		ImGui::AlignmentGroup("MiddleLeftGroup", AlignX::Left, AlignY::Middle, [&]() { ImGui::Button("o", btnSize); }, mOffsets[3]);
		ImGui::SameLine();
		ImGui::AlignmentGroup("MiddleCenterGroup", AlignX::Center, AlignY::Middle, [&]()
			{ 
				ImGui::Button("o", btnSize);
				if (mShowMultiElementAlignment)
				{
					ImGui::SameLine(); ImGui::Button("x", btnSize);
					ImGui::Button("1", btnSize); ImGui::SameLine(); ImGui::Button("2", btnSize);
				}
			},
			mOffsets[4]);
		ImGui::SameLine();
		ImGui::AlignmentGroup("MiddleRightGroup", AlignX::Right, AlignY::Middle, [&]() { ImGui::Button("o", btnSize); }, mOffsets[5]);

		ImGui::Spacing();

		ImGui::AlignmentGroup("BottomLeftGroup", AlignX::Left, AlignY::Bottom, [&]() { ImGui::Button("o", btnSize); }, mOffsets[6]);
		ImGui::SameLine();
		ImGui::AlignmentGroup("BottomCenterGroup", AlignX::Center, AlignY::Bottom, [&]() { ImGui::Button("o", btnSize); }, mOffsets[7]);
		ImGui::SameLine();
		ImGui::AlignmentGroup("BottomRightGroup", AlignX::Right, AlignY::Bottom, [&]() { ImGui::Button("o", btnSize); }, mOffsets[8]);
	}
	ImGui::EndChild();
	if (mRemovedWindowPadding)
	{
		ImGui::PopStyleVar();
	}
	ImGui::Spacing();

	ImGui::Checkbox("Remove Window Padding", &mRemovedWindowPadding);

	ImGui::Checkbox("Modify Offsets", &mModifyOffset);

	ImGui::Checkbox("Show Multi-Element Alignment", &mShowMultiElementAlignment);

	if (mModifyOffset)
	{
		mWasModifyingOffset = true;
		for (int i = 0; i < 9; ++i)
		{
			ImGui::SliderFloat2(("Offset " + std::to_string(i)).c_str(), (float*)&mOffsets[i], -50.0f, 50.0f);
		}
	}
	else if (mWasModifyingOffset)
	{
		for (int i = 0; i < 9; ++i)
		{
			mOffsets[i] = ImVec2(0.0f, 0.0f);
		}
		mWasModifyingOffset = false;
	}

	ImGui::Spacing();

	ImGui::TextWrapped("The three alignment helpers (CenterAlignedGroup, RightAlignedGroup, and AlignmentGroup) use a two-pass system to measure the size of their contents and then position them accordingly. They use a simple caching mechanism to avoid re-measuring every frame.");
	ImGui::TextWrapped("The AlignmentGroup function is the most flexible, allowing you to specify both horizontal and vertical alignment within the window's content region. The CenterAlignedGroup and RightAlignedGroup are convenience wrappers for common use cases.");
	ImGui::TextWrapped("Feel free to resize the window to see how the alignment adapts dynamically.");


}


