#pragma once

#include <string>
#include <imgui.h>

enum class ImGuiImageFit
{
    STRETCH,    // Fill requested size, may distort
    CONTAIN,    // Fit inside size, preserve aspect ratio
    COVER,      // Crop to fill size, preserve aspect ratio
    CUSTOM_UV   // Use cfg.uv0 / cfg.uv1 directly
};

struct ImGuiImageConfig
{
    ImGuiImageFit fit = ImGuiImageFit::STRETCH;

	// only used if fit is set to CUSTOM_UV
    ImVec2 uv0 = ImVec2(0.0f, 0.0f);
    ImVec2 uv1 = ImVec2(1.0f, 1.0f);

    ImVec4 tint_col = ImVec4(1, 1, 1, 1);
    ImVec4 bg_col = ImVec4(0, 0, 0, 0);
	float border_thickness = 0.0f;
	ImVec4 border_col = ImVec4(0, 0, 0, 0);

    bool debug = false;
    bool preserve_aspect = true;
};

namespace ImGui
{
    void DrawTexture(const std::string& path, ImGuiImageConfig cfg, ImVec2 size = {});
    inline void DrawTexture(const std::string& path, ImVec2 size = {})
    {
        DrawTexture(path, ImGuiImageConfig{}, size);
	}

    void CleanTexture(const std::string& path);

    void CleanAllTextures();
}