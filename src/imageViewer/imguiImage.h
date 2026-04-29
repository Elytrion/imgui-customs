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
    /*
	    Draws an image from a file path. The image is cached by path and reused after first load.
	    If size is (0,0), the original image size is used. Otherwise, the image is drawn according to cfg.fit with the requested size.
	    If the image is updated on disk, call CleanTexture(path) to clear the cache for that image and force reload on next DrawTexture call.
    */
    void DrawTexture(const std::string& path, ImGuiImageConfig cfg, ImVec2 size = {});
    inline void DrawTexture(const std::string& path, ImVec2 size = {})
    {
        DrawTexture(path, ImGuiImageConfig{}, size);
	}

	// Clears the cached texture for the given path. Must be called before the OpenGL context is destroyed.
	// Optional to call when an image file is updated on disk and needs to be reloaded.
    void CleanTexture(const std::string& path);

	// Clears all cached textures. Must be called at least once before the OpenGL context is destroyed.
    void CleanAllTextures();
}