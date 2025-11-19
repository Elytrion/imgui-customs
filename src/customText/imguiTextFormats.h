#pragma once
#include <string>
#include <imgui.h>
#include <imgui_internal.h>

#ifndef IM_FMTARGS
#define IM_FMTARGS(FMT) __attribute__((format(printf, FMT, FMT+1)))
#define IM_FMTLIST(FMT)
#endif

// ImGui namespace access for convenience
namespace ImGui
{
	static std::string EllipsizeRightFit(const char* text, float max_width)
	{
		if (!text) return {};
		if (max_width <= 0.0f) return "...";

		ImVec2 full = ImGui::CalcTextSize(text);
		if (full.x <= max_width) return text;

		const char* dots = "...";
		float dots_w = ImGui::CalcTextSize(dots).x;
		float target_w = max_width - dots_w;
		if (target_w <= 0.0f) return std::string(dots);

		const char* p = text;
		const char* last_good = text;
		float w = 0.0f;
		while (*p)
		{
			const char* prev = p;
			unsigned char c = (unsigned char)*p++;
			if (c & 0x80) while ((*p & 0xC0) == 0x80) p++; // advance over UTF-8 continuation bytes

			w += ImGui::CalcTextSize(prev, p).x;
			if (w > target_w) break;
			last_good = p;
		}
		std::string out(text, last_good);
		out += dots;
		return out;
	}
	static std::string EllipsizeLeftFit(const char* text, float max_width)
	{
		if (!text) return {};
		if (max_width <= 0.0f) return "...";

		ImVec2 full = ImGui::CalcTextSize(text);
		if (full.x <= max_width) return text;

		const char* dots = "...";
		float dots_w = ImGui::CalcTextSize(dots).x;
		float target_w = max_width - dots_w;
		if (target_w <= 0.0f) return std::string(dots);

		const char* begin = text;
		const char* end = text;
		while (*end) end++;

		const char* first_to_keep = end;
		float w = 0.0f;
		while (first_to_keep > begin)
		{
			const char* p = first_to_keep;
			do { --p; } while (p > begin && ((*p & 0xC0) == 0x80)); // step back one UTF-8 codepoint

			w += ImGui::CalcTextSize(p, first_to_keep).x;
			if (w > target_w) break;
			first_to_keep = p;
		}
		return std::string("...") + std::string(first_to_keep, end);
	}
	void TextLimited(const char* text, bool cut_left = false, float max_width = -1.0f, float padding_px = 0.0f, bool show_tooltip = true)
	{
		float avail = (max_width >= 0.0f ? max_width : ImGui::GetContentRegionAvail().x) - padding_px;
		if (avail < 0.0f) avail = 0.0f;
		std::string s = cut_left ? EllipsizeLeftFit(text, avail) : EllipsizeRightFit(text, avail);

		ImGui::TextUnformatted(s.c_str());
		if (show_tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
		{
			ImGui::BeginTooltip();
			float tooltipWrapWidth = (avail > 0.0f) ? avail : ImGui::GetFontSize() * 50.0f;
			ImGui::PushTextWrapPos(tooltipWrapWidth);
			ImGui::TextUnformatted(text);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
	void TextLimitedV(bool cut_left, float max_width, float padding_px, bool show_tooltip, const char* fmt, va_list args)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		const char* text, * text_end;
		ImFormatStringToTempBufferV(&text, &text_end, fmt, args);
		TextLimited(text, cut_left, max_width, padding_px, show_tooltip);
	}
	void TextLimitedF(bool cut_left, float max_width, float padding_px, bool show_tooltip, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		TextLimitedV(cut_left, max_width, padding_px, show_tooltip, fmt, args);
		va_end(args);
	}
	void TextLimitedF(const char* fmt, ...) // default to right cut, full width, no padding, with tooltip
	{
		va_list args;
		va_start(args, fmt);
		TextLimitedV(false, -1.0f, 0.0f, true, fmt, args);
		va_end(args);
	}
}