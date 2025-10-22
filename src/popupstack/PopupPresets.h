#pragma once
namespace PopupPreset //non constexpr to play nice with ImVec2
{
    inline PopupConfig Percent(float w, float h, PopupPosition pos = PopupPosition::CENTERED)
    {
        PopupConfig c{};
        c.size_mode = PopupSizeMode::PERCENT;
        c.size = ImVec2(w, h);
        c.position = pos;
        return c;
    }

    inline PopupConfig Pixels(float w, float h, PopupPosition pos = PopupPosition::CENTERED)
    {
        PopupConfig c{};
        c.size_mode = PopupSizeMode::PIXELS;
        c.size = ImVec2(w, h);
        c.position = pos;
        return c;
    }

    inline PopupConfig Auto(PopupPosition pos = PopupPosition::CENTERED)
    {
        PopupConfig c{};
	    // remove NoResize flag
		c.flags &= ~ImGuiWindowFlags_NoResize;
		c.flags |= ImGuiWindowFlags_AlwaysAutoResize;
        c.size_mode = PopupSizeMode::AUTO;
        c.position = pos;
        return c;
    }

    inline PopupConfig WithPos(const PopupConfig& base, PopupPosition pos)
    {
        PopupConfig c = base;
        c.position = pos;
        return c;
    }

    // ------------------------
    // Base sizes (percent)
    // ------------------------
    inline PopupConfig TINY = Percent(0.15f, 0.00f);  // 15% width, auto height
    inline PopupConfig SMALL = Percent(0.30f, 0.00f);  // 30% width, auto height
    inline PopupConfig MEDIUM = Percent(0.50f, 0.00f);
    inline PopupConfig LARGE = Percent(0.70f, 0.00f);
    inline PopupConfig XLARGE = Percent(0.85f, 0.00f);
    inline PopupConfig FULL = Percent(0.98f, 0.98f);  // nearly fullscreen modal

    // Common corner/center combos
    inline PopupConfig TINY_CENTER = WithPos(TINY, PopupPosition::CENTERED);
    inline PopupConfig TINY_TL = WithPos(TINY, PopupPosition::TOP_LEFT);
    inline PopupConfig TINY_TR = WithPos(TINY, PopupPosition::TOP_RIGHT);
    inline PopupConfig TINY_BL = WithPos(TINY, PopupPosition::BOTTOM_LEFT);
    inline PopupConfig TINY_BR = WithPos(TINY, PopupPosition::BOTTOM_RIGHT);

    inline PopupConfig SMALL_CENTER = WithPos(SMALL, PopupPosition::CENTERED);
    inline PopupConfig SMALL_TL = WithPos(SMALL, PopupPosition::TOP_LEFT);
    inline PopupConfig SMALL_TR = WithPos(SMALL, PopupPosition::TOP_RIGHT);
    inline PopupConfig SMALL_BL = WithPos(SMALL, PopupPosition::BOTTOM_LEFT);
    inline PopupConfig SMALL_BR = WithPos(SMALL, PopupPosition::BOTTOM_RIGHT);

    inline PopupConfig MEDIUM_CENTER = WithPos(MEDIUM, PopupPosition::CENTERED);
    inline PopupConfig MEDIUM_TL = WithPos(MEDIUM, PopupPosition::TOP_LEFT);
    inline PopupConfig MEDIUM_TR = WithPos(MEDIUM, PopupPosition::TOP_RIGHT);
    inline PopupConfig MEDIUM_BL = WithPos(MEDIUM, PopupPosition::BOTTOM_LEFT);
    inline PopupConfig MEDIUM_BR = WithPos(MEDIUM, PopupPosition::BOTTOM_RIGHT);

    inline PopupConfig LARGE_CENTER = WithPos(LARGE, PopupPosition::CENTERED);
    inline PopupConfig LARGE_TL = WithPos(LARGE, PopupPosition::TOP_LEFT);
    inline PopupConfig LARGE_TR = WithPos(LARGE, PopupPosition::TOP_RIGHT);
    inline PopupConfig LARGE_BL = WithPos(LARGE, PopupPosition::BOTTOM_LEFT);
    inline PopupConfig LARGE_BR = WithPos(LARGE, PopupPosition::BOTTOM_RIGHT);

    inline PopupConfig XLARGE_CENTER = WithPos(XLARGE, PopupPosition::CENTERED);

    // ------------------------
    // Layout-style presets
    // ------------------------

    // Tall sidebars (mostly vertical)
    // Takes ~30% width on the side, ~90% of viewport height.
    inline PopupConfig TALL_LEFT = Percent(0.30f, 0.90f, PopupPosition::TOP_LEFT);
    inline PopupConfig TALL_RIGHT = Percent(0.30f, 0.90f, PopupPosition::TOP_RIGHT);

    // Wide strips at the top/bottom (mostly horizontal)
    // Spans ~90% of width, ~30% of height.
    inline PopupConfig WIDE_TOP = Percent(0.90f, 0.30f, PopupPosition::TOP_LEFT);
    inline PopupConfig WIDE_BOTTOM = Percent(0.90f, 0.30f, PopupPosition::BOTTOM_LEFT);
    // Alias if you prefer the abbreviated name
    inline PopupConfig WIDE_BTM = WIDE_BOTTOM;

    // Common “sheet” style dialogs centered
    inline PopupConfig SHEET_MD = Percent(0.60f, 0.70f, PopupPosition::CENTERED);
    inline PopupConfig SHEET_LG = Percent(0.75f, 0.80f, PopupPosition::CENTERED);

    // Split-like panels
    inline PopupConfig SPLIT_LEFT = Percent(0.50f, 0.90f, PopupPosition::TOP_LEFT);
    inline PopupConfig SPLIT_RIGHT = Percent(0.50f, 0.90f, PopupPosition::TOP_RIGHT);

    // Toasts / notifications (small, corner pinned)
    inline PopupConfig TOAST_TR = Percent(0.25f, 0.00f, PopupPosition::TOP_RIGHT);
    inline PopupConfig TOAST_BR = Percent(0.25f, 0.00f, PopupPosition::BOTTOM_RIGHT);

    // ------------------------
    // Pixel-based handy presets
    // ------------------------
    // Classic dialogs – fixed width, auto height
    inline PopupConfig DIALOG_360 = Pixels(360.0f, 0.0f, PopupPosition::CENTERED);
    inline PopupConfig DIALOG_480 = Pixels(480.0f, 0.0f, PopupPosition::CENTERED);
    inline PopupConfig DIALOG_640 = Pixels(640.0f, 0.0f, PopupPosition::CENTERED);

    // Fixed boxes
    inline PopupConfig BOX_400x300 = Pixels(400.0f, 300.0f, PopupPosition::CENTERED);
    inline PopupConfig BOX_640x480 = Pixels(640.0f, 480.0f, PopupPosition::CENTERED);

    // Pixel-sized toasts
    inline PopupConfig TOAST_PX_TR = Pixels(320.0f, 0.0f, PopupPosition::TOP_RIGHT);
    inline PopupConfig TOAST_PX_BR = Pixels(320.0f, 0.0f, PopupPosition::BOTTOM_RIGHT);

    // ------------------------
    // Auto-sized (content-driven) convenience
    // ------------------------
    inline PopupConfig AUTO_CENTER = Auto(PopupPosition::CENTERED);
    inline PopupConfig AUTO_TL = Auto(PopupPosition::TOP_LEFT);
    inline PopupConfig AUTO_TR = Auto(PopupPosition::TOP_RIGHT);
    inline PopupConfig AUTO_BL = Auto(PopupPosition::BOTTOM_LEFT);
    inline PopupConfig AUTO_BR = Auto(PopupPosition::BOTTOM_RIGHT);
}
