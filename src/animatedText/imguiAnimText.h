#pragma once
#include <string>
#include <imgui.h>
#include <imgui_internal.h>

namespace Noise
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

}

// ImGui namespace access for convenience
namespace ImGui
{
    // amp: vertical wobble in pixels (also used as vertical padding)
    // freq: waves across the whole string
    // speed: cycles per second
    void TextWobble(const char* text, float amp = 3.0f, float freq = 1.5f, float speed = 1.0f, ImU32 col = IM_COL32_WHITE)
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
    void TextShaky(const char* text,
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
            float jx = (Noise::ValueNoise1D(sx, t, speed, roughness) * 2.0f - 1.0f) * spread_x;
            float jy = (Noise::ValueNoise1D(sy, t, speed * 1.13f, roughness) * 2.0f - 1.0f) * spread_y;

            // Draw this character only
            dl->AddText(font, size, ImVec2(pen.x + jx, pen.y + jy), col, p0, p);

            // Advance by measuring this glyph (no FindGlyph needed)
            ImVec2 w = font->CalcTextSizeA(size, FLT_MAX, 0.0f, p0, p);
            pen.x += w.x;

            ++i;
        }
    }

}