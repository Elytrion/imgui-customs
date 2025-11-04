#pragma once
#include <string>
#include <imgui.h>
#include <imgui_internal.h>

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
}