#pragma once
#include <string>
#include <utility>
#include <imgui.h>
#include "windowStore/imguiWindowStore.h"
// you can replace window store with a global cache or directly using ImGui::GetStateStorage()

enum class AlignX { Left, Center, Right };
enum class AlignY { Top, Middle, Bottom };


namespace ImGui
{
	// If you already have a WindowStore + Has/SetVec2, use that.
	// This function anchors a group in the window content region.
	template<typename Widgets>
	void AlignmentGroup(const char* id,
		AlignX ax, AlignY ay,
		Widgets&& widgets,
		ImVec2 offset = ImVec2(0, 0),
		bool restore_cursor_after = true,
		const char* cache_tag = "ag_size")
	{
		const ImVec2 cursor_before = ImGui::GetCursorPos();

		ImGuiID base = ImGui::GetID(id);
		auto store = ImGui::GetWindowStore(base);

		const bool newPass = !store.Has(cache_tag);
		const ImVec2 size = store.GetVec2(cache_tag, ImVec2(0, 0));

		if (newPass)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.f);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushItemFlag(ImGuiItemFlags_AllowOverlap, true);
			ImGui::PushItemFlag(ImGuiItemFlags_NoWindowHoverableCheck, true);
			ImGui::PushItemFlag(ImGuiItemFlags_NoNavDisableMouseHover, true);
		}

		const ImVec2 cr_min = ImGui::GetWindowContentRegionMin();
		const ImVec2 cr_max = ImGui::GetWindowContentRegionMax();

		const float anchor_x =
			(ax == AlignX::Left) ? cr_min.x :
			(ax == AlignX::Center) ? (cr_min.x + cr_max.x) * 0.5f :
			cr_max.x;

		const float anchor_y =
			(ay == AlignY::Top) ? cr_min.y :
			(ay == AlignY::Middle) ? (cr_min.y + cr_max.y) * 0.5f :
			cr_max.y;

		const float pivot_x =
			(ax == AlignX::Left) ? 0.0f :
			(ax == AlignX::Center) ? 0.5f :
			1.0f;

		const float pivot_y =
			(ay == AlignY::Top) ? 0.0f :
			(ay == AlignY::Middle) ? 0.5f :
			1.0f;

		ImGui::SetCursorPos(ImVec2(anchor_x - size.x * pivot_x + offset.x,
			anchor_y - size.y * pivot_y + offset.y));

		ImGui::PushID(id);
		ImGui::BeginGroup();
		widgets();
		ImGui::EndGroup();
		ImGui::PopID();

		if (newPass)
		{
			ImGui::PopItemFlag();
			ImGui::PopItemFlag();
			ImGui::PopItemFlag();
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		store.SetVec2(cache_tag, ImGui::GetItemRectSize());

		if (restore_cursor_after)
			ImGui::SetCursorPos(cursor_before);
	}

	template<typename Widgets>
	void RightAlignedGroup(const char* id, Widgets&& widgets, float offsetX = 0, float offsetY = 0)
	{
		ImGuiID base = ImGui::GetID(id);
		auto store = ImGui::GetWindowStore(base);

		bool newPass = !store.Has("ral_group_size");
		ImVec2 widgetSpace = store.GetVec2("ral_group_size", ImVec2(0, 0));

		if (newPass)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.f);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushItemFlag(ImGuiItemFlags_AllowOverlap, true);
			ImGui::PushItemFlag(ImGuiItemFlags_NoWindowHoverableCheck, true);
			ImGui::PushItemFlag(ImGuiItemFlags_NoNavDisableMouseHover, true);
		}

		ImGui::SetCursorPosX(
			std::max(
				ImGui::GetStyle().ItemSpacing.x,
				ImGui::GetWindowContentRegionMax().x - widgetSpace.x - offsetX
			)
		);
		if (offsetY != 0) {
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
		}

		ImGui::BeginGroup();
		widgets();
		ImGui::EndGroup();

		if (newPass)
		{
			ImGui::PopItemFlag();
			ImGui::PopItemFlag();
			ImGui::PopItemFlag();
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		store.SetVec2("ral_group_size", ImGui::GetItemRectSize());
	}

	template<typename Widgets>
	void CenterAlignedGroup(const char* id, Widgets&& widgets, float offsetX = 0, float offsetY = 0)
	{
		ImGuiID base = ImGui::GetID(id);
		auto store = ImGui::GetWindowStore(base);

		const bool  newPass = !store.Has("cal_group_size");
		const ImVec2 widgetSpace = store.GetVec2("cal_group_size", ImVec2(0, 0));

		if (newPass)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.f);
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushItemFlag(ImGuiItemFlags_AllowOverlap, true);
			ImGui::PushItemFlag(ImGuiItemFlags_NoWindowHoverableCheck, true);
			ImGui::PushItemFlag(ImGuiItemFlags_NoNavDisableMouseHover, true);
		}

		const ImVec2 cr_min = ImGui::GetWindowContentRegionMin();
		const ImVec2 cr_max = ImGui::GetWindowContentRegionMax();
		const float content_left = cr_min.x;
		const float content_right = cr_max.x;
		const float content_center = (content_left + content_right) * 0.5f;

		float x = content_center - (widgetSpace.x * 0.5f) + offsetX;

		// clamp to not go past left padding
		x = std::max(x, content_left + ImGui::GetStyle().ItemSpacing.x);

		ImGui::SetCursorPosX(x);

		if (offsetY != 0.0f)
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

		ImGui::BeginGroup();
		widgets();
		ImGui::EndGroup();

		if (newPass)
		{
			ImGui::PopItemFlag(); 
			ImGui::PopItemFlag(); 
			ImGui::PopItemFlag(); 
			ImGui::PopItemFlag(); 
			ImGui::PopStyleVar();
		}

		store.SetVec2("cal_group_size", ImGui::GetItemRectSize());
	}


}
