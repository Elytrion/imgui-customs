#pragma once
#include <string>
#include <imgui.h>
#include <imgui_internal.h>

namespace
{
struct TextWrappedLimitedCacheEntry
{
	float wrap_width = 0.0f;
	float max_height = 0.0f;
	std::string input;
	std::string output;
	std::string cutoff;
	int last_frame_used = 0;
};
}

typedef int TextLimitedFlags;
enum TextLimitedFlags_
{
	TextLimitedFlags_NoTooltip = 0,				 // dont show tooltip
	TextLimitedFlags_TooltipShowAll = 1 << 0,	 // tooltip shows full text (default)
	TextLimitedFlags_TooltipShowCutoff = 1 << 1, // tooltip shows only truncated text
};

// ImGui namespace access for convenience
namespace ImGui
{
	static float MINIMUM_TOOLTIP_WIDTH_MULTIPLIER = 20.0f; // modify this to change the minimum width of the tooltip in terms of characters
	static std::string EllipsizeRightFit(const char* text, float max_width, std::string* out_cutoff = nullptr)
	{
		if (out_cutoff) out_cutoff->clear();
		const char* dots = "...";

		if (!text) return {};
		if (max_width <= 0.0f) return dots;

		ImVec2 full = ImGui::CalcTextSize(text);
		if (full.x <= max_width) return text;

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

		const char* text_end = text + strlen(text);

		if (out_cutoff && last_good < text_end)
			*out_cutoff = std::string(last_good, text_end); // hidden suffix

		std::string out(text, last_good);
		out += dots;
		return out;
	}
	static std::string EllipsizeLeftFit(const char* text, float max_width, std::string* out_cutoff = nullptr)
	{
		if (out_cutoff) out_cutoff->clear();
		const char* dots = "...";

		if (!text) return {};
		if (max_width <= 0.0f) return dots;

		ImVec2 full = ImGui::CalcTextSize(text);
		if (full.x <= max_width) return text;

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

		if (out_cutoff && first_to_keep > begin)
			*out_cutoff = std::string(begin, first_to_keep); // hidden prefix

		return std::string("...") + std::string(first_to_keep, end);
	}

	// Draw text that is limited to a maximum width, adding "..." as needed.
	void TextLimited(
		const char* text,
		bool cut_left = false,
		float max_width = -1.0f,
		float padding_px = 0.0f,
		TextLimitedFlags flags = TextLimitedFlags_TooltipShowAll)
	{
		float avail = (max_width >= 0.0f ? max_width : ImGui::GetContentRegionAvail().x) - padding_px;
		std::string coff;
		if (avail < 0.0f) avail = 0.0f;
		std::string s = cut_left ? EllipsizeLeftFit(text, avail, &coff) : EllipsizeRightFit(text, avail, &coff);

		const bool truncated = !coff.empty();
		ImGui::TextUnformatted(s.c_str());

		bool want_tooltip = (flags & (TextLimitedFlags_TooltipShowAll | TextLimitedFlags_TooltipShowCutoff)) != 0;

		if (want_tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
		{
			std::string tooltip_text;
			if (truncated && (flags & TextLimitedFlags_TooltipShowCutoff))
			{
				tooltip_text = (cut_left) ? coff + "..." : "..." + coff;
			}
			else if (flags & TextLimitedFlags_TooltipShowAll)
			{
				tooltip_text = text;
			}

			ImGui::BeginTooltip();
			const float minWidth = ImGui::GetFontSize() * MINIMUM_TOOLTIP_WIDTH_MULTIPLIER;
			float tooltipWrapWidth = (avail > minWidth) ? avail : minWidth;
			ImGui::PushTextWrapPos(tooltipWrapWidth);
			ImGui::TextUnformatted(tooltip_text.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
	void TextLimitedV(bool cut_left, float max_width, float padding_px, TextLimitedFlags flags, const char* fmt, va_list args)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		const char* text, * text_end;
		ImFormatStringToTempBufferV(&text, &text_end, fmt, args);
		TextLimited(text, cut_left, max_width, padding_px, flags);
	}
	void TextLimitedF(bool cut_left, float max_width, float padding_px, TextLimitedFlags flags, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		TextLimitedV(cut_left, max_width, padding_px, flags, fmt, args);
		va_end(args);
	}
	// Draw formatted text that is limited to a maximum width, adding "..." as needed. Defaults to right cut, full width, no padding, with tooltip.
	void TextLimitedF(const char* fmt, ...) // default to right cut, full width, no padding, with tooltip
	{
		va_list args;
		va_start(args, fmt);
		TextLimitedV(false, -1.0f, 0.0f, TextLimitedFlags_TooltipShowAll, fmt, args);
		va_end(args);
	}

	static std::string EllipsizeMultilineFit(const char* text, float wrap_width, float max_height, std::string* out_cutoff = nullptr)
	{
		const char* dots = "...";
		if (out_cutoff) out_cutoff->clear();
		if (!text)
			return {};
		if (wrap_width <= 0.0f || max_height <= 0.0f)
			return dots;

		// First see if we need to truncate at all
		ImVec2 full = ImGui::CalcTextSize(text, nullptr, false, wrap_width);
		if (full.y <= max_height)
			return text;

		// Build a table of UTF-8 codepoint boundaries so we never split a codepoint
		std::vector<const char*> cps;
		cps.reserve(strlen(text) + 1);

		const char* p = text;
		while (*p)
		{
			cps.push_back(p);
			unsigned char c = (unsigned char)*p++;
			if (c & 0x80)        // multibyte
				while ((*p & 0xC0) == 0x80) // continuation bytes
					p++;
		}
		const char* text_end = p;
		cps.push_back(text_end); // sentinel end

		const int cp_count = (int)cps.size() - 1; // last index with an actual codepoint
		if (cp_count <= 0)
			return dots;

		// Binary search the largest prefix that still fits when we append "..."
		int lo = 0;
		int hi = cp_count; // we know full text doesn't fit, but it's ok for the search

		while (lo < hi)
		{
			int mid = (lo + hi + 1) / 2; // bias upward

			std::string tmp(text, cps[mid]);
			tmp += dots;

			ImVec2 sz = ImGui::CalcTextSize(tmp.c_str(), nullptr, false, wrap_width);
			if (sz.y <= max_height)
				lo = mid;   // this fits, try to take more
			else
				hi = mid - 1; // too tall, take less
		}

		if (lo <= 0)
			return dots; // not even one codepoint + "..." fits nicely

		if (out_cutoff && cps[lo] < text_end)
			*out_cutoff = std::string(cps[lo], text_end); // hidden tail

		std::string out(text, cps[lo]);
		out += dots;
		return out;
	}
	
	// Draw wrapped text that is limited to a maximum height, adding "..." as needed.
	inline void TextWrappedLimited(
		const char* itemID, // must be provided to have unique cache entries for different text instances, since text may change
		const char* text,
		float max_width = -1.0f,
		float max_height_px = -1.0f, // max_height_px <= 0 -> use max_lines * line_height
		int   max_lines = 5,
		float padding_px = 0.0f,
		TextLimitedFlags flags = TextLimitedFlags_TooltipShowAll)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		if (!text)
			text = "";

		// Compute available wrap width 
		float avail_w = (max_width >= 0.0f ? max_width : ImGui::GetContentRegionAvail().x) - padding_px;
		if (avail_w < 0.0f)
			avail_w = 0.0f;

		// Compute max height in pixels
		if (max_height_px <= 0.0f)
		{
			float line_h = ImGui::GetTextLineHeight();
			max_height_px = line_h * (float)max_lines;
		}

		// ---------- CACHING LAYER ----------
		// one cache map per frame
		static std::unordered_map<ImGuiID, TextWrappedLimitedCacheEntry> s_cache;
		int frame = ImGui::GetFrameCount();
		// Use a widget ID as cache key.
		ImGuiID id = window->GetID(itemID);
		TextWrappedLimitedCacheEntry& entry = s_cache[id];
		entry.last_frame_used = frame;
		bool need_recalc =
			entry.input != text ||
			fabsf(entry.wrap_width - avail_w) > 0.5f ||
			fabsf(entry.max_height - max_height_px) > 0.5f;
		std::string coff;
		if (need_recalc)
		{
			entry.input = text;
			entry.wrap_width = avail_w;
			entry.max_height = max_height_px;
			entry.output = EllipsizeMultilineFit(text, avail_w, max_height_px, &coff);
			entry.cutoff = coff;
		}
		else
			coff = entry.cutoff;

		if (s_cache.size() > 128) // arbitrary max cache size for cleanup
		{
			for (auto it = s_cache.begin(); it != s_cache.end(); )
			{
				if (it->second.last_frame_used < frame - 300) // not used for ~300 frames
					it = s_cache.erase(it);
				else
					++it;
			}
		}

		const std::string& s = entry.output;
		const bool truncated = !coff.empty();
		// ---------- END CACHING LAYER ----------

		// Render it with wrapping
		float wrap_pos = ImGui::GetCursorPos().x + avail_w; // window-local
		ImGui::PushTextWrapPos(wrap_pos);
		ImGui::TextUnformatted(s.c_str());
		ImGui::PopTextWrapPos();

		bool want_tooltip = (flags & (TextLimitedFlags_TooltipShowAll | TextLimitedFlags_TooltipShowCutoff)) != 0;

		// Tooltip with full text
		if (want_tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
		{
			std::string tooltip_text;
			if (truncated && (flags & TextLimitedFlags_TooltipShowCutoff))
			{
				tooltip_text = "..." + coff;
			}
			else if (flags & TextLimitedFlags_TooltipShowAll)
			{
				tooltip_text = text;
			}

			ImGui::BeginTooltip();
			const float minWidth = ImGui::GetFontSize() * MINIMUM_TOOLTIP_WIDTH_MULTIPLIER;
			float tooltipWrapWidth = (avail_w > minWidth) ? avail_w : minWidth;
			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + tooltipWrapWidth);
			ImGui::TextUnformatted(tooltip_text.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
}