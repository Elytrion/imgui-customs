#pragma once
#include <string>
#include <utility>
#include <imgui.h>
#include <imgui_internal.h>

enum class AlignX { Left, Center, Right };
enum class AlignY { Top, Middle, Bottom };

namespace
{
	inline ImGuiID AlignBaseKey(const char* id)
	{
		return ImGui::GetID(id);
	}
	inline ImGuiID AlignKey(ImGuiID base, const char* tag)
	{
		// Hash the tag under base key scope (no allocations, no temporary strings)
		return ImHashStr(tag, 0, base);
	}
	inline ImGuiID AlignPresentKey(ImGuiID base, const char* tag)
	{
		ImGuiID k = AlignKey(base, tag);
		return ImHashStr("##__present", 0, k);
	}
	inline bool AlignHas(ImGuiStorage* st, ImGuiID base, const char* tag)
	{
		return st->GetBool(AlignPresentKey(base, tag), false);
	}
	inline void AlignMarkPresent(ImGuiStorage* st, ImGuiID base, const char* tag)
	{
		st->SetBool(AlignPresentKey(base, tag), true);
	}
	inline ImVec2 AlignGetVec2(ImGuiStorage* st, ImGuiID base, const char* tag, ImVec2 def = ImVec2(0, 0))
	{
		ImGuiID k = AlignKey(base, tag);
		ImGuiID kx = ImHashStr("##x", 0, k);
		ImGuiID ky = ImHashStr("##y", 0, k);
		return ImVec2(st->GetFloat(kx, def.x), st->GetFloat(ky, def.y));
	}
	inline void AlignSetVec2(ImGuiStorage* st, ImGuiID base, const char* tag, ImVec2 v)
	{
		ImGuiID k = AlignKey(base, tag);
		ImGuiID kx = ImHashStr("##x", 0, k);
		ImGuiID ky = ImHashStr("##y", 0, k);
		st->SetFloat(kx, v.x);
		st->SetFloat(ky, v.y);
		AlignMarkPresent(st, base, tag);
	}
	inline void AlignClear(ImGuiStorage* st, ImGuiID base, const char* tag)
	{
		st->SetBool(AlignPresentKey(base, tag), false);
	}
}

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
		ImGuiStorage* st = ImGui::GetStateStorage();
		const ImGuiID base = AlignBaseKey(id);
		const bool newPass = !AlignHas(st, base, cache_tag);
		const ImVec2 size = AlignGetVec2(st, base, cache_tag, ImVec2(0, 0));

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

		AlignSetVec2(st, base, cache_tag, ImGui::GetItemRectSize());

		if (restore_cursor_after)
			ImGui::SetCursorPos(cursor_before);
	}

	template<typename Widgets>
	void RightAlignedGroup(const char* id, Widgets&& widgets, float offsetX = 0, float offsetY = 0)
	{
		const char* cache_tag = "ral_group_size";
		ImGuiStorage* st = ImGui::GetStateStorage();
		const ImGuiID base = AlignBaseKey(id);
		const bool newPass = !AlignHas(st, base, cache_tag);
		const ImVec2 widgetSpace = AlignGetVec2(st, base, cache_tag, ImVec2(0, 0));

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

		AlignSetVec2(st, base, cache_tag, ImGui::GetItemRectSize());
	}

	template<typename Widgets>
	void CenterAlignedGroup(const char* id, Widgets&& widgets, float offsetX = 0, float offsetY = 0)
	{
		const char* cache_tag = "ral_group_size";
		ImGuiStorage* st = ImGui::GetStateStorage();
		const ImGuiID base = AlignBaseKey(id);
		const bool newPass = !AlignHas(st, base, cache_tag);
		const ImVec2 widgetSpace = AlignGetVec2(st, base, cache_tag, ImVec2(0, 0));

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

		AlignSetVec2(st, base, cache_tag, ImGui::GetItemRectSize());
	}

	struct HorizontalMidGroup
	{
		ImGuiStorage* St = nullptr;
		ImGuiID Base = 0;

		ImVec2 StartCursor{};
		float SpacingX = 0.0f;

		float CachedRowH = 0.0f;
		float MeasuredRowH = 0.0f;
		float X = 0.0f;

		bool Begun = false;

		// Tags (hashed under Base)
		static constexpr const char* kRowH = "##hmg_row_h"; // stored as Vec2(y = height)

		explicit HorizontalMidGroup(const char* row_id)
		{
			Base = AlignBaseKey(row_id);
			St = ImGui::GetStateStorage();

			StartCursor = ImGui::GetCursorPos();
			SpacingX = ImGui::GetStyle().ItemSpacing.x;

			// Read cached row height (we store it in Vec2.y)
			if (AlignHas(St, Base, kRowH))
				CachedRowH = AlignGetVec2(St, Base, kRowH, ImVec2(0, 0)).y;
			else
				CachedRowH = 0.0f; // first frame settles

			ImGui::PushID(Base);
			ImGui::BeginGroup(); // row occupies space as a whole
			Begun = true;
		}

		~HorizontalMidGroup()
		{
			if (Begun)
				End();
		}

		void End()
		{
			ImGui::EndGroup();
			ImGui::PopID();
			Begun = false;

			if (MeasuredRowH > 0.0f)
				AlignSetVec2(St, Base, kRowH, ImVec2(0.0f, MeasuredRowH));
		}

		template <class WidgetFn>
		HorizontalMidGroup& DrawWidget(const char* item_tag, WidgetFn&& fn)
		{
			// Per-item cached size
			const ImVec2 cached = AlignGetVec2(St, Base, item_tag, ImVec2(0, 0));

			// Y offset computed from cached row height and cached item height
			const float y_off = (CachedRowH > 0.0f) ? (CachedRowH - cached.y) * 0.5f : 0.0f;

			// Place explicitly inside the row group (avoid SameLine)
			ImGui::SetCursorPos(ImVec2(StartCursor.x + X, StartCursor.y + y_off));

			ImGui::PushID(item_tag);
			ImGui::BeginGroup();
			fn();
			ImGui::EndGroup();
			ImGui::PopID();

			const ImVec2 measured = ImGui::GetItemRectSize();

			// Update caches for next frame
			AlignSetVec2(St, Base, item_tag, measured);

			// Track row max height for next frame
			if (measured.y > MeasuredRowH)
				MeasuredRowH = measured.y;

			// Advance X for next widget
			X += measured.x + SpacingX;

			return *this;
		}
	};
}
