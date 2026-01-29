#pragma once
#include <string>
#include <utility>
#include <imgui.h>
#include "windowStore/imguiWindowStore.h"
// you can replace window store with a global cache or directly using ImGui::GetStateStorage()
// see imguiAlignment_noStore.h for an example without window store

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

		const ImVec2 scroll(ImGui::GetScrollX(), ImGui::GetScrollY());

		const ImVec2 cursorPos =
			ImVec2(anchor_x - size.x * pivot_x + offset.x + scroll.x * 2.f,
				anchor_y - size.y * pivot_y + offset.y + scroll.y * 2.f);

		ImGui::SetCursorPos(cursorPos);
		ImGui::Dummy(ImVec2(0, 0)); // to make sure the cursor position is updated before BeginGroup
		ImGui::SetCursorPos(cursorPos); // need to set again after Dummy to avoid it consuming the pos set
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

	struct HorizontalMidGroup
	{
		ImGuiID        Base = 0;
		ImGui::WindowStore Store = ImGui::WindowStore(nullptr, 0);

		ImVec2 StartCursor{};
		float  SpacingX = 0.0f;

		float  CachedRowH = 0.0f;
		float  MeasuredRowH = 0.0f;
		float  X = 0.0f;

		bool   Begun = false;
		static constexpr const char* kRowH = "hmg_row_h";

		explicit HorizontalMidGroup(const char* row_id)
			: Base(ImGui::GetID(row_id))
			, Store(ImGui::GetWindowStore(Base))
		{
			StartCursor = ImGui::GetCursorPos();
			SpacingX = ImGui::GetStyle().ItemSpacing.x;

			CachedRowH = Store.GetFloat(kRowH, 0.0f);

			ImGui::PushID(Base);
			ImGui::BeginGroup(); // whole row is a group (so it occupies space properly)
			Begun = true;
		}

		~HorizontalMidGroup()
		{
			if (Begun)
				End();
		}

		void End()
		{
			// Close the row group and commit cache.
			ImGui::EndGroup();
			ImGui::PopID();
			Begun = false;

			// Store row max height for next frame (avoid storing 0 if nothing drawn).
			if (MeasuredRowH > 0.0f)
				Store.SetFloat(kRowH, MeasuredRowH);
		}

		template <class WidgetFn>
		HorizontalMidGroup& DrawWidget(const char* item_tag, WidgetFn&& fn, float width_offset = 0.0f)
		{
			// Advance X by any specified width offset
			X += width_offset;
			// Per-item cache tag
			// (we keep it simple: item_tag -> Vec2 size)
			const char* size_tag = item_tag; // use item_tag directly for Vec2 cache

			const ImVec2 cached = Store.GetVec2(size_tag, ImVec2(0, 0));

			// Compute Y offset using cached row height (1-frame delay).
			// If cache is empty (0), offset will be 0 (first frame "settles").
			const float y_off = (CachedRowH > 0.0f) ? (CachedRowH - cached.y) * 0.5f : 0.0f;

			// Position inside the row group explicitly
			ImGui::SetCursorPos(ImVec2(StartCursor.x + X, StartCursor.y + y_off));

			ImGui::PushID(item_tag);
			ImGui::BeginGroup();
			fn();
			ImGui::EndGroup();
			ImGui::PopID();

			const ImVec2 measured = ImGui::GetItemRectSize();

			// Update caches for next frame
			Store.SetVec2(size_tag, measured);

			// Track true row height for next frame
			if (measured.y > MeasuredRowH)
				MeasuredRowH = measured.y;

			// Advance X for next widget
			X += measured.x + SpacingX;

			return *this;
		}
	};

}
