#pragma once
#include <string>
#include <imgui.h>
#include <imgui_internal.h>

namespace AnimHelpers
{
    // 1D value noise: random samples per "tick", smooth-interpolated in between.
    // speed = "hops per second". roughness in [0..1]: 0 = linear, 1 = smoothstep. (>1 makes it even smoother)
    static inline float ValueNoise1D(uint32_t seed, float t, float speed, float roughness = 1.0f) {
        // Tiny stable hash -> [0,1)
        auto Hash01 = [&](uint32_t x) {
            x ^= x * 0x27d4eb2d;
            x ^= x >> 15; x *= 0x85ebca6b; x ^= x >> 13; x *= 0xc2b2ae35; x ^= x >> 16;
            return (x & 0x00FFFFFF) * (1.0f / 16777216.0f);
            };

        float u = t * speed;
        float k = floorf(u);
        float f = u - k;
        float a = Hash01(seed + (uint32_t)k);
        float b = Hash01(seed + (uint32_t)k + 1u);
        // smoother interpolation if roughness>0
        float w = (roughness <= 0.0f) ? f : (roughness >= 1.0f ? (f * f * (3.0f - 2.0f * f))  // smoothstep
            : ((1.0f - roughness) * f + roughness * (f * f * (3.0f - 2.0f * f))));
        return a + (b - a) * w; // 0..1
    }

    // --- helpers ---
    static inline ImU32 LerpRGBA(ImU32 a, ImU32 b, float t)
    {
        ImVec4 ca = ImGui::ColorConvertU32ToFloat4(a);
        ImVec4 cb = ImGui::ColorConvertU32ToFloat4(b);
        ImVec4 cc = ImLerp(ca, cb, ImClamp(t, 0.0f, 1.0f));
        return ImGui::ColorConvertFloat4ToU32(cc);
    }

    static inline ImU32 LerpHSV(ImU32 a, ImU32 b, float t)
    {
        auto u32_to_hsva = [](ImU32 c, float& h, float& s, float& v, float& a_out) {
            ImVec4 cf = ImGui::ColorConvertU32ToFloat4(c);
            ImGui::ColorConvertRGBtoHSV(cf.x, cf.y, cf.z, h, s, v);
            a_out = cf.w;
            };
        float h1, s1, v1, a1, h2, s2, v2, a2;
        u32_to_hsva(a, h1, s1, v1, a1);
        u32_to_hsva(b, h2, s2, v2, a2);
        // Shortest hue interpolation
        float dh = h2 - h1;
        if (dh > 0.5f)  dh -= 1.0f;
        if (dh < -0.5f) dh += 1.0f;
        float h = h1 + dh * t;
        if (h < 0.0f) h += 1.0f;
        if (h > 1.0f) h -= 1.0f;
        float s = ImLerp(s1, s2, t);
        float v = ImLerp(v1, v2, t);
        float alpha = ImLerp(a1, a2, t);
        ImVec4 rgb; ImGui::ColorConvertHSVtoRGB(h, s, v, rgb.x, rgb.y, rgb.z); rgb.w = alpha;
        return ImGui::ColorConvertFloat4ToU32(rgb);
    }

    static inline ImU32 SampleStops(const ImU32* stops, int count, float u, bool use_hsv)
    {
        if (count <= 0) return IM_COL32_WHITE;
        if (count == 1) return stops[0];

        // u in [0,1] across the gradient; supports repeating if caller wraps it.
        u = ImClamp(u, 0.0f, 1.0f);
        float x = u * (count - 1);
        int i = (int)floorf(x);
        if (i >= count - 1) return stops[count - 1];
        float t = x - i;
        return use_hsv ? LerpHSV(stops[i], stops[i + 1], t)
            : LerpRGBA(stops[i], stops[i + 1], t);
    }
}

// ImGui namespace access for convenience
namespace ImGui
{
	// Wobbling text effect using sine waves.
    // amp: vertical wobble in pixels (also used as vertical padding)
    // freq: waves across the whole string
    // speed: cycles per second
    inline void TextWobble(const char* text, float amp = 3.0f, float freq = 1.5f, float speed = 1.0f, ImU32 col = IM_COL32_WHITE)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();

        // Reserve layout so following items won’t overlap
        const ImVec2 top_left = ImGui::GetCursorScreenPos();
        const ImVec2 text_size = ImGui::CalcTextSize(text);          // single-line
        const float  pad_top = amp;                                 // how far wobble can go upward
        const float  pad_bottom = amp;                              // how far wobble can go downward
        ImGui::Dummy(ImVec2(text_size.x, text_size.y + pad_top + pad_bottom));

        // Draw baseline at the padded position (so wobble stays within the reserved box)
        ImVec2 draw_pos = ImVec2(top_left.x, top_left.y + pad_top);
        const int vtx0 = dl->VtxBuffer.Size;
        dl->AddText(draw_pos, col, text);

		// Modify vertices to create wobble effect
        const float t = (float)ImGui::GetTime();
		const float two_pi = 6.28319f; // approx 2*PI, i don't really care about precision here
        const float wavelength = (text_size.x > 0.0f) ? (two_pi * freq / text_size.x) : 0.0f;
        const float temporal = two_pi * speed * t;
		// Iterate vertices of the last added text
        for (int i = vtx0; i < dl->VtxBuffer.Size; ++i)
        {
            ImDrawVert& v = dl->VtxBuffer[i];
            float spatial = (v.pos.x - draw_pos.x) * wavelength;
            v.pos.y += amp * sinf(spatial + temporal);
        }
    }

    // Jittery text where each character moves independently inside a rectangle set by spread.
    // spread_x/spread_y: max displacement in px (envelope size).
    // speed: how quickly each char meanders (steps/sec).
    // chaos: how different neighbors are (bigger = more desync).
    // roughness: 0=linear ping-pong between samples (sharper moves), 1=smoothstep (smoother).
    inline void TextShaky(const char* text,
        float spread_x = 1.5f, float spread_y = 3.0f,
        float speed = 12.0f, float chaos = 1.0f, float roughness = 0.9f,
        ImU32 col = IM_COL32_WHITE)
    {

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImFont* font = ImGui::GetFont();
        float       size = ImGui::GetFontSize();

        // Reserve enough vertical space so jitter won't overlap other widgets.
        ImVec2 top_left = ImGui::GetCursorScreenPos();
        ImVec2 base_sz = ImGui::CalcTextSize(text);
        ImGui::Dummy(ImVec2(base_sz.x + spread_x * 2.0f, base_sz.y + spread_y * 2.0f));

        ImVec2 pen = ImVec2(top_left.x + spread_x, top_left.y + spread_y);

        const float t = (float)ImGui::GetTime();

        const char* p = text;
        unsigned int cp = 0;
        int i = 0;

        while (*p)
        {
            const char* p0 = p;
            int len = ImTextCharFromUtf8(&cp, p, nullptr);
            if (len <= 0) break;
            p += len;

            if (cp == '\n') {
                pen.x = top_left.x + spread_x;
                pen.y += ImGui::GetTextLineHeightWithSpacing();
                continue;
            }

            // Per-character seeds (desync neighbors; chaos scales the seed spacing)
            uint32_t sx = (uint32_t)(0x9E3779B9u * (i + 17)) ^ (uint32_t)cp;
            uint32_t sy = (uint32_t)(0x85EBCA6Bu * (i + 31)) ^ ((uint32_t)cp * 0xC2B2AE35u);
            sx += (uint32_t)(chaos * 997.0f);
            sy += (uint32_t)(chaos * 1511.0f);

            // Value noise in [-1,1], scaled by spread
            float jx = (AnimHelpers::ValueNoise1D(sx, t, speed, roughness) * 2.0f - 1.0f) * spread_x;
            float jy = (AnimHelpers::ValueNoise1D(sy, t, speed * 1.13f, roughness) * 2.0f - 1.0f) * spread_y;

            // Draw this character only
            dl->AddText(font, size, ImVec2(pen.x + jx, pen.y + jy), col, p0, p);

            // Advance by measuring this glyph (no FindGlyph needed)
            ImVec2 w = font->CalcTextSizeA(size, FLT_MAX, 0.0f, p0, p);
            pen.x += w.x;

            ++i;
        }
    }


    inline void TextGradientEx(const char* text,
        const ImU32* stops, int stop_count,
		int mode = 0, // 0 == none, 1 == repeat, 2 == pingpong
        float phase_speed = 0.5f,   // “widths per second”
        float phase_offset = 0.0f,  // start phase [0..1)
        bool use_hsv = false,       // interpolate in HSV?
        bool smooth_pingpong_peaks = true)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size = ImGui::CalcTextSize(text);

        // Layout like normal text
        ImGui::Dummy(size);

        const int v0 = dl->VtxBuffer.Size;
        dl->AddText(pos, IM_COL32_WHITE, text); // color gets overridden

        // Find horizontal span of just emitted vertices
        float minx = FLT_MAX, maxx = -FLT_MAX;
        for (int i = v0; i < dl->VtxBuffer.Size; ++i) {
            float x = dl->VtxBuffer[i].pos.x;
            if (x < minx) minx = x;
            if (x > maxx) maxx = x;
        }
        float span = ImMax(1.0f, maxx - minx);

        // Phase (animation)
        float phase = phase_offset;
        if (mode != 0 && phase_speed != 0.0f) {
            float t = (float)ImGui::GetTime();
            phase += t * phase_speed;  // measured in “widths”
        }

        auto wrap_repeat = [](float x) { x -= floorf(x); return x; }; // [0,1)
        auto wrap_pingpong = [smooth_pingpong_peaks](float x) {
            float s = x - floorf(x);              // saw in [0,1)
            float tri = 1.0f - fabsf(2.0f * s - 1); // 0->1->0
            if (smooth_pingpong_peaks)            // soften corners
                tri = tri * tri * (3.0f - 2.0f * tri);
            return tri;                           // still [0,1]
            };

        // Recolor
        for (int i = v0; i < dl->VtxBuffer.Size; ++i) {
            float u = (dl->VtxBuffer[i].pos.x - minx) / span; // 0..1 across text
            float s = u + phase;

            float tval;
            if (mode == 0) {
                tval = ImClamp(u, 0.0f, 1.0f);
            }
            else if (mode == 1) {
                tval = wrap_repeat(s);
            }
            else { // PingPong
                tval = wrap_pingpong(s);
            }

            // Preserve original alpha of the vertex (multiply final alpha by it)
            ImU32 c = AnimHelpers::SampleStops(stops, stop_count, tval, use_hsv);
            ImU32 vcol = dl->VtxBuffer[i].col;
            float a_vtx = ((vcol >> 24) & 0xFF) / 255.0f;
            ImVec4 cf = ImGui::ColorConvertU32ToFloat4(c);
            cf.w *= a_vtx;
            dl->VtxBuffer[i].col = ImGui::ColorConvertFloat4ToU32(cf);
        }
    }

	// Displays text with a color gradient applied.
    inline void TextGradient(const char* text, const ImU32* stops, int stop_count)
    {
        TextGradientEx(text, stops, stop_count, 0, 0, 0);
	}
	// Displays text with an animated color gradient applied.
    inline void TextGradientAnimated(const char* text, const ImU32* stops, int stop_count,
        bool pingpong = true, float phase_speed = 1.0f, float phase_offset = 0.0f)
    {
        TextGradientEx(text, stops, stop_count, (pingpong ? 2 : 1), phase_speed, phase_offset);
	}


    inline std::string GetAnimatedDots(int dotCount = 3, float secondsPerStep = 0.3f)
    {
        int dots = static_cast<int>(ImGui::GetTime() / secondsPerStep) % (dotCount + 1);
        return std::string(dots, '.');
    }
    inline void TextWithAnimatedDots(const char* text, int dotCount = 3, float secondsPerStep = 0.3f)
    {
        std::string dots = GetAnimatedDots(dotCount, secondsPerStep);
        ImGui::TextUnformatted((std::string(text) + dots).c_str());
    }


    inline void TextMarquee(
        const char* str_id,
        const char* text,
        float width = 0.0f,
        float speed = 50.0f,
        bool right_to_left = true)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        // Calculate text size
        ImVec2 text_size = ImGui::CalcTextSize(text, nullptr, false);
        if (text_size.x <= 0.0f)
            text_size.x = 1.0f; // avoid divide-by-zero / modulo issues

        // Determine visible width
        float region_w = (width > 0.0f) ? width : ImGui::GetContentRegionAvail().x;
        if (region_w <= 0.0f)
            region_w = 1.0f;

        // Total ticker item size (height similar to a regular text line with padding)
        ImVec2 item_size(region_w, text_size.y + style.FramePadding.y * 2.0f);

        ImGuiID id = window->GetID(str_id);
        ImRect bb(window->DC.CursorPos,
            { window->DC.CursorPos.x + item_size.x, window->DC.CursorPos.y + item_size.y });

        ImGui::ItemSize(item_size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return;

        ImGuiIO& io = ImGui::GetIO();

        // Use ImGui's state storage for per-widget offset
        ImGuiStorage* storage = ImGui::GetStateStorage();
        float offset = storage->GetFloat(id, 0.0f);

        // Advance offset using deltaTime
        offset += io.DeltaTime * speed;
        // Distance before we wrap around: text width + region width
        float full_width = text_size.x + region_w;
        if (full_width <= 0.0f)
            full_width = 1.0f;

        // Wrap offset to [0, full_width)
        if (offset > full_width)
            offset = fmodf(offset, full_width);
        storage->SetFloat(id, offset);

        // Compute base X so that text "enters" from one side
        float base_x;
        if (right_to_left)
        {
            // Start just off the right side and move left
            base_x = bb.Min.x + region_w - offset;
        }
        else
        {
            // Start just off the left side and move right
            base_x = bb.Min.x - text_size.x + offset;
        }

        float text_y = bb.Min.y + style.FramePadding.y;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Clip to the ticker rect so text doesn't overflow
        ImGui::PushClipRect(bb.Min, bb.Max, true);

        ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);

        // First instance
        draw_list->AddText(ImVec2(base_x, text_y), col_text, text);

        // Second instance, shifted by full_width, so scrolling feels continuous
        float base_x2 = right_to_left ? (base_x + full_width) : (base_x - full_width);
        draw_list->AddText(ImVec2(base_x2, text_y), col_text, text);

        ImGui::PopClipRect();
    }

}