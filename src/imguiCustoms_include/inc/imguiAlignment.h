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
	static inline ImU32 WithAlphaMul(ImU32 col, float a01)
	{
		ImVec4 c = ImGui::ColorConvertU32ToFloat4(col);
		c.w *= (a01 < 0.f ? 0.f : a01 > 1.f ? 1.f : a01);
		return ImGui::ColorConvertFloat4ToU32(c);
	}
}

namespace ImGui
{
	/*
	@brief Aligns a group of widgets according to the specified alignment anchors.
	@details Takes in a callable 'widgets' that contains the ImGui calls to be aligned. These widgets are
			 measured on the first pass (when they are invisible) and then positioned accordingly on subsequent passes.
			 These widgets are aligned to the window content region, so use child windows if you want to align within a specific area.

	@param id			- A unique identifier for the alignment group.
	@param ax			- Horizontal alignment anchor (Left, Center, Right).
	@param ay			- Vertical alignment anchor (Top, Middle, Bottom).
	@param widgets		- A callable (e.g., lambda) that contains the ImGui calls to be aligned.
	@param offset		- Optional offset to apply to the aligned position.
	@param restore_cursor_after - If true, restores the cursor position after rendering the aligned group. Best to leave true.
	@param cache_tag	- A tag to identify the cached size of this group. Change if the contents change dynamically.
	@return				- The start position where the aligned group was rendered.
	*/
	template<typename Widgets>
	ImVec2 AlignmentGroup(const char* id,
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

		return cursorPos;
	}

	/*
	@brief Retrieves the cached size of an alignment group.
	@param id			- The unique identifier for the alignment group.
	@param cache_tag	- The tag used to identify the cached size [Must be the same as the one used in AlignmentGroup].
	@return				- The cached size of the alignment group, or (0,0) if not cached.
	*/
	inline ImVec2 GetAlignmentGroupSize(const char* id, const char* cache_tag = "ag_size")
	{
		ImGuiStorage* st = ImGui::GetStateStorage();
		const ImGuiID base = AlignBaseKey(id);
		return AlignGetVec2(st, base, cache_tag, ImVec2(0, 0));
	}

	/*
	@brief Aligns a group of widgets to the right side of the window content region.

	@param id			- A unique identifier for the alignment group.
	@param widgets		- A callable (e.g., lambda) that contains the ImGui calls to be aligned.
	@param offsetX		- Optional horizontal offset to apply to the aligned position.
	@param offsetY		- Optional vertical offset to apply to the aligned position.
	*/
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

	/*
	@brief Aligns a group of widgets to the center of the window content region.

	@param id			- A unique identifier for the alignment group.
	@param widgets		- A callable (e.g., lambda) that contains the ImGui calls to be aligned.
	@param offsetX		- Optional horizontal offset to apply to the aligned position.
	@param offsetY		- Optional vertical offset to apply to the aligned position.
	*/
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

	/*
	@brief Scoped helper class to layout widgets horizontally with vertical mid alignment.
	@details Usage:
				{
					HorizontalMidGroup hmg("unique_row_id");
					hmg.DrawWidget("item1", []() { ImGui::Button("Button 1"); });
					hmg.DrawWidget("item2", []() { ImGui::Text("Some text"); });
					// ...
				} // automatic End() on destruction, or manually call hmg.End();
	*/
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

		/*
		@brief Ends the horizontal mid group. Automatically called on destruction, but can be called manually if needed.
		*/
		void End()
		{
			ImGui::EndGroup();
			ImGui::PopID();
			Begun = false;

			if (MeasuredRowH > 0.0f)
				AlignSetVec2(St, Base, kRowH, ImVec2(0.0f, MeasuredRowH));
		}

		/*
		@brief Draws a widget within the horizontal mid group, aligning it vertically to the midpoint of the tallest widget in the row.
		@param item_tag			- A unique tag for this widget, used for caching its size. Change if the widget's size changes dynamically.
		@param fn				- A callable (e.g., lambda) that contains the ImGui calls to draw the widget.
		@param width_offset		- Optional horizontal offset to apply before drawing this widget. Useful for fine-tuning spacing.
		@return					- Reference to this HorizontalMidGroup for function chaining.
		*/
		template <class WidgetFn>
		HorizontalMidGroup& DrawWidget(const char* item_tag, WidgetFn&& fn, float width_offset = 0.0f)
		{
			// Advance X by any specified width offset
			X += width_offset;
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


	// ------ Specialized versions of AlignmentGroup ------- (Not shown in examples)

	/*
	@brief Aligns a group of widgets according to the specified alignment anchors, with a background rectangle.
	@details This is a specialized version of AlignmentGroup that draws a background rectangle behind the aligned widgets.
			 The background is drawn using ImDrawList channels to ensure it appears behind the widgets.
			 The size of the background is determined by the cached size of the alignment group.
	@param id			- A unique identifier for the alignment group.
	@param ax			- Horizontal alignment anchor (Left, Center, Right).
	@param ay			- Vertical alignment anchor (Top, Middle, Bottom).
	@param widgets		- A callable (e.g., lambda) that contains the ImGui calls to be aligned.
	@param bgPadding	- Optional padding to add around the background rectangle.
	@param offset		- Optional offset to apply to the aligned position.
	@param bgCol		- Color of the background rectangle (default is ImGuiCol_WindowBg).
	@param restore_cursor_after - If true, restores the cursor position after rendering the aligned group. Best to leave true.
	@param cache_tag	- A tag to identify the cached size of this group. Change if the contents change dynamically.
	@return				- The start position where the aligned group was rendered.
	*/
	template<typename Widgets>
	ImVec2 AlignmentGroupBG(const char* id,
		AlignX ax, AlignY ay,
		Widgets&& widgets,
		ImVec2 bgPadding = ImVec2(5, 5),
		ImVec2 offset = ImVec2(0, 0),
		ImU32 bgCol = ImGui::GetColorU32(ImGuiCol_WindowBg),
		bool restore_cursor_after = true,
		const char* cache_tag = "ag_size")
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->ChannelsSplit(2);
		draw_list->ChannelsSetCurrent(1);
		ImVec2 ag_pos = ImGui::AlignmentGroup(id, ax, ay, std::forward<Widgets>(widgets), offset, restore_cursor_after, cache_tag);
		ImVec2 size = ImGui::GetAlignmentGroupSize(id);
		draw_list->ChannelsSetCurrent(0);
		// Convert window-local to screen
		ImVec2 p0 = ImGui::GetWindowPos() + ag_pos - ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
		ImVec2 p1 = p0 + size;
		ImGui::GetWindowDrawList()->AddRectFilled(
			p0 - bgPadding,
			p1 + bgPadding,
			bgCol
		);
		draw_list->ChannelsMerge();
		return ag_pos;
	}

	/*
	@brief Helper function to draw a faded rectangle with a solid portion and a gradient portion.
	@param dl			- The ImDrawList to draw on.
	@param p0			- The top-left corner of the rectangle.
	@param p1			- The bottom-right corner of the rectangle.
	@param col			- The base color of the rectangle (alpha will be modified by alpha parameter).
	@param alpha		- Overall alpha multiplier for the rectangle (0.0f = fully transparent, 1.0f = fully opaque).
	@param gradPct		- The percentage of the rectangle's height that should be a gradient (0.0f to 1.0f).
	@param solidPct		- The percentage of the rectangle's height that should be solid (0.0f to 1.0f). The solid portion is drawn at the bottom.
	*/
	static void DrawFadedRect(ImDrawList* dl,
		ImVec2 p0, ImVec2 p1,
		ImU32 col,
		float alpha,   // t
		float gradPct, // g
		float solidPct // b
	)
	{
		if (alpha <= 0.0f)
			return;

		gradPct = ImClamp(gradPct, 0.0f, 1.0f);
		solidPct = ImClamp(solidPct, 0.0f, 1.0f);

		if (gradPct + solidPct <= 0.0f)
			return;

		if (gradPct + solidPct > 1.0f)
			gradPct = 1.0f - solidPct;

		const ImU32 solidCol = WithAlphaMul(col, alpha);
		const ImU32 clearCol = WithAlphaMul(col, 0.0f);

		ImVec2 size{ p1.x - p0.x, p1.y - p0.y };

		// Solid
		if (solidPct > 0.0f)
		{
			ImVec2 s0(p0.x, p1.y - size.y * solidPct);
			ImVec2 s1(p1.x, p1.y);
			dl->AddRectFilled(s0, s1, solidCol);
		}
		// Gradient
		if (gradPct > 0.0f)
		{
			float y1 = p1.y - size.y * solidPct;
			float y0 = y1 - size.y * gradPct;

			dl->AddRectFilledMultiColor(
				ImVec2(p0.x, y0), ImVec2(p1.x, y1),
				clearCol, clearCol,
				solidCol, solidCol
			);
		}
	}

	/*
	@brief Aligns a group of widgets according to the specified alignment anchors, with a background rectangle that has a gradient fade effect.
	@details This is a specialized version of AlignmentGroup that draws a background rectangle with a gradient fade effect behind the aligned widgets.
			 The background is drawn using ImDrawList channels to ensure it appears behind the widgets.
			 The size of the background is determined by the cached size of the alignment group.
	@param id			- A unique identifier for the alignment group.
	@param ax			- Horizontal alignment anchor (Left, Center, Right).
	@param ay			- Vertical alignment anchor (Top, Middle, Bottom).
	@param widgets		- A callable (e.g., lambda) that contains the ImGui calls to be aligned.
	@param g			- The percentage of the background's height that should be a gradient fade (0.0f to 1.0f).
	@param b			- The percentage of the background's height that should be solid (0.0f to 1.0f). The solid portion is drawn at the bottom.
	@param bgPadding	- Optional padding to add around the background rectangle.
	@param offset		- Optional offset to apply to the aligned position.
	@param bgCol		- Color of the background rectangle (default is ImGuiCol_WindowBg).
	@param restore_cursor_after - If true, restores the cursor position after rendering the aligned group. Best to leave true.
	@param cache_tag	- A tag to identify the cached size of this group. Change if the contents change dynamically.
	*/
	template<typename Widgets>
	ImVec2 AlignmentGroupBG_Gradient(const char* id,
		AlignX ax, AlignY ay,
		Widgets&& widgets,
		float g, // gradient pct
		float b, // solid pct
		ImVec2 bgPadding = ImVec2(5, 5),
		ImVec2 offset = ImVec2(0, 0),
		ImU32 bgCol = ImGui::GetColorU32(ImGuiCol_WindowBg),
		bool restore_cursor_after = true,
		const char* cache_tag = "ag_size")
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->ChannelsSplit(2);
		draw_list->ChannelsSetCurrent(1);

		ImVec2 ag_pos = ImGui::AlignmentGroup(
			id, ax, ay, std::forward<Widgets>(widgets), offset, restore_cursor_after, cache_tag);

		ImVec2 size = ImGui::GetAlignmentGroupSize(id, cache_tag);

		draw_list->ChannelsSetCurrent(0);

		// if size isn't known yet skip drawing
		if (size.x > 0.0f || size.y > 0.0f)
		{
			ImVec2 p0 = ImGui::GetWindowPos() + ag_pos - ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
			ImVec2 p1 = p0 + size;

			p0 -= bgPadding;
			p1 += bgPadding;

			DrawFadedRect(
				draw_list,
				p0, p1,
				bgCol,
				ImGui::GetStyle().Alpha, // alpha
				g, // gradient pct
				b  // solid pct
			);
		}

		draw_list->ChannelsMerge();
		return ag_pos;
	}
}
