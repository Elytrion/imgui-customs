#pragma once
#include <string>
#include <imgui.h>

namespace
{
	static ImVec2 ClampToWorkArea(const ImVec2& anchor_pos, const ImVec2& size, const ImVec2& pivot, const ImGuiViewport* vp)
	{
		const ImVec2 work_min = vp->WorkPos;
		const ImVec2 work_max = ImVec2(vp->WorkPos.x + vp->WorkSize.x,
			vp->WorkPos.y + vp->WorkSize.y);
	
		// Convert anchor+pivot into actual rect
		ImVec2 rect_min = ImVec2(
			anchor_pos.x - pivot.x * size.x,
			anchor_pos.y - pivot.y * size.y);
	
		ImVec2 rect_max = ImVec2(
			rect_min.x + size.x,
			rect_min.y + size.y);
	
		ImVec2 delta = ImVec2(0.0f, 0.0f);
	
		if (rect_max.x > work_max.x) delta.x -= (rect_max.x - work_max.x);
		if (rect_max.y > work_max.y) delta.y -= (rect_max.y - work_max.y);
		if (rect_min.x < work_min.x) delta.x += (work_min.x - rect_min.x);
		if (rect_min.y < work_min.y) delta.y += (work_min.y - rect_min.y);
	
		// Apply same correction back to anchor point
		return ImVec2(anchor_pos.x + delta.x, anchor_pos.y + delta.y);
	}
}

namespace ImGui
{
	struct CustomTooltipConfig
	{
		ImVec2 pivot		{ -1,-1 }; // Pivot point for tooltip positioning (-1,-1 means stay on cursor), pivot is relative to the item rect (0..1)
		ImVec2 cursorPivot	{ 0,0 }; // if cursor mode, pivot relative to cursor pos (0..1)
		ImVec2 offset		{ 0,0 }; // Offset from pivot point

		ImVec2 padding		{ 3,3 }; // Padding inside tooltip
		float rounding		{ -1.f }; // Tooltip corner rounding (-1 = full rounding)

		ImU32   bgCol = IM_COL32(50, 50, 50, 230); // Background color
		ImU32   borderCol = IM_COL32(255, 255, 255, 200); // Border color
		ImU32   textCol = IM_COL32(255, 255, 255, 255); // Text color

		bool keepWithinWorkArea = true; // Clamp tooltip to work area

		ImGuiHoveredFlags hoverFlags = ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNormal;
		ImGuiTooltipFlags tooltipFlags = 0;
		ImGuiWindowFlags  additionalWindowFlags = 0;
	};

	// Call this immediately after an item to show a custom tooltip when hovered.
	void DrawCustomTooltip(const char* tooltipTxt, const CustomTooltipConfig& cfg = {}, bool enabled = true)
	{
		if (!enabled || !tooltipTxt || tooltipTxt[0] == '\0')
			return;
		if (!IsItemHovered(cfg.hoverFlags))
			return;

		const ImGuiViewport* vp = ImGui::GetWindowViewport(); // usually correct for the hovered item
		const ImVec2 item_min = ImGui::GetItemRectMin();
		const ImVec2 item_max = ImGui::GetItemRectMax();
		const ImVec2 item_size = { item_max.x - item_min.x, item_max.y - item_min.y };

		const ImVec2 text_sz = ImGui::CalcTextSize(tooltipTxt);
		const ImVec2 pad = cfg.padding;
		const ImVec2 tip_sz = ImVec2(text_sz.x + pad.x * 2.0f, text_sz.y + pad.y * 2.0f);
	
		ImVec2 pos;
		ImVec2 tooltipPivot = cfg.cursorPivot;
		bool cursor_mode = (cfg.pivot.x < 0.0f && cfg.pivot.y < 0.0f);
		if (cursor_mode)
		{
			pos.x = ImGui::GetMousePos().x + cfg.offset.x;
			pos.y = ImGui::GetMousePos().y + cfg.offset.y;
		}
		else
		{
			// pivot is normalized relative to item rect (0..1)
			const ImVec2 anchor = { item_min.x + item_size.x * cfg.pivot.x, item_min.y + item_size.y * cfg.pivot.y };
			pos.x = anchor.x + cfg.offset.x;
			pos.y = anchor.y + cfg.offset.y;
			tooltipPivot = { 1 - cfg.pivot.x, 1 - cfg.pivot.y }; // inverse pivot for tooltip
		}
		if (cfg.keepWithinWorkArea)
			pos = ClampToWorkArea(pos, tip_sz, tooltipPivot, vp);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, cfg.padding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, (cfg.rounding < 0.0f) ? ImGui::GetStyle().WindowRounding : cfg.rounding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

		ImGui::PushStyleColor(ImGuiCol_PopupBg, cfg.bgCol);
		ImGui::PushStyleColor(ImGuiCol_Border, cfg.borderCol);
		ImGui::PushStyleColor(ImGuiCol_Text, cfg.textCol);

		ImGuiID hovered_id = ImGui::GetItemID();
		ImGui::PushID(hovered_id);

		ImGui::SetNextWindowPos(pos, ImGuiCond_Always, tooltipPivot);
		ImGui::BeginTooltipEx(cfg.tooltipFlags, cfg.additionalWindowFlags);
		ImGui::TextUnformatted(tooltipTxt);
		ImGui::EndTooltip();

		ImGui::PopID();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar(3);
	}

	void DrawCustomTooltip(const char* tooltipTxt, ImVec2 pivot, ImVec2 offset, bool enabled = true)
	{
		CustomTooltipConfig cfg;
		cfg.pivot = pivot;
		cfg.offset = offset;
		DrawCustomTooltip(tooltipTxt, cfg, enabled);
	}
}
