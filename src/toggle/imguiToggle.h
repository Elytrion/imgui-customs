#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui
{
	struct ToggleConfig
	{
		ImVec2   size = ImVec2(0, 0);						            // (0,0) means “auto” from frame-height
		float    rounding = -1.0f;							            // -1 = half-height
		float    handle_rounding = -1.0f;							    // -1 = circlular handle >= 0 rounded rect
		float    padding = 1.5f;								        // space between handle & box
		ImU32    col_off_bg = IM_COL32(0xD9, 0xD9, 0xD9, 255);	        // off color for the background
		ImU32    col_on_bg = IM_COL32(0x8E, 0xD3, 0x56, 255);	        // on color for the background
		ImU32    col_off_hover_bg = IM_COL32(0xC8, 0xC8, 0xC8, 255);    // hover color for the background when toggle is off
		ImU32    col_on_hover_bg = IM_COL32(0xA0, 0xC8, 0x44, 255);	    // hover color for the background when toggle is on
		ImU32    col_off_hnd = IM_COL32(255, 255, 255, 255);	        // off color for the handle
		ImU32    col_on_hnd = IM_COL32(255, 255, 255, 255);	            // on color for the handle
		ImU32    col_off_hover_hnd = IM_COL32(240, 240, 240, 255);	    // hover color for the handle when toggle is off
		ImU32    col_on_hover_hnd = IM_COL32(240, 240, 240, 255);	    // hover color for the handle when toggle is on
		float  (*easingFunc)(float) = nullptr;						    // easing function for the animation, nullptr = linear
		float    anim_speed = 0.08f;						            // seconds to full transition
	};
	// Toggle a switch with the given ID and value pointer. The toggle will animate between on/off states,
	// and can be customized with the ToggleConfig struct. A default ToggleConfig is used if none is provided.
	void Toggle(const char* str_id, bool* state, const ToggleConfig& cfg = {})
	{
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2      p = ImGui::GetCursorScreenPos();

		float size_x = cfg.size.x > 0 ? cfg.size.x : ImGui::GetContentRegionAvail().x;
		float size_y = cfg.size.y > 0 ? cfg.size.y : ImGui::GetFrameHeight();

        ImGui::InvisibleButton(str_id, {size_x, size_y});
        if (ImGui::IsItemClicked())
            *state = !*state;

        // derive geometry
        float h = cfg.size.y > 0 ? cfg.size.y : ImGui::GetFrameHeight();
        float w = cfg.size.x > 0 ? cfg.size.x : (h * 2.f);
        float r = cfg.rounding >= 0 ? cfg.rounding : (h * 0.5f);
        float pad = cfg.padding;

        float bg_r = cfg.rounding >= 0 ? cfg.rounding : (h * 0.5f);
        float handle_half = (h * 0.5f) - pad;
        float hnd_r = cfg.handle_rounding >= 0 ? cfg.handle_rounding : handle_half;

        // animation
        ImGuiContext& g = *GImGui;
        float t = *state ? 1.0f : 0.0f;
        if (g.LastActiveId == g.CurrentWindow->GetID(str_id))
        {
            float norm = ImSaturate(g.LastActiveIdTimer / cfg.anim_speed);
            float eased = cfg.easingFunc ? cfg.easingFunc(norm) : norm;
            t = *state ? eased : (1.0f - eased);
        }

        // pick colors
        bool hovered = ImGui::IsItemHovered();

        ImVec4 off_bg = ImGui::ColorConvertU32ToFloat4(hovered ? cfg.col_off_hover_bg : cfg.col_off_bg);
        ImVec4 on_bg = ImGui::ColorConvertU32ToFloat4(hovered ? cfg.col_on_hover_bg : cfg.col_on_bg);
        ImVec4 off_hnd = ImGui::ColorConvertU32ToFloat4(hovered ? cfg.col_off_hover_hnd : cfg.col_off_hnd);
        ImVec4 on_hnd = ImGui::ColorConvertU32ToFloat4(hovered ? cfg.col_on_hover_hnd : cfg.col_on_hnd);
        // lerp colors
        ImVec4 lerped_bg = ImLerp(off_bg, on_bg, t);
        ImVec4 lerped_hnd = ImLerp(off_hnd, on_hnd, t);
        // back to U32
        ImU32 col_bg = ImGui::GetColorU32(lerped_bg);
        ImU32 col_handle = ImGui::GetColorU32(lerped_hnd);

        // background
        dl->AddRectFilled(p,
            ImVec2(p.x + w, p.y + h),
            col_bg, r);

        // handle
        ImVec2 h_min = {
        p.x + (h * 0.5f) - handle_half + t * ((w - h)),
        p.y + pad
        };
        ImVec2 h_max = {
            h_min.x + handle_half * 2,
            p.y + h - pad
        };
        dl->AddRectFilled(
            h_min, h_max,
            ImGui::GetColorU32(col_handle),
            hnd_r);
	}
}
