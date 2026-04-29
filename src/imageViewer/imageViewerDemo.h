#pragma once

#include "imguiImage.h"
#include "demo_module.h"

#include <array>
#include <cstdint>
#include <stb_image.h>

class ImageViewerDemo : public DemoModule
{
public:
    ImageViewerDemo() : DemoModule("Image Viewer", "Image Viewer Panel") {}

    void OnCleanup() override
    {
		// Must be called before the OpenGL context is destroyed
        ImGui::CleanAllTextures();
    }

protected:
    void DrawSelectedDemo() override;
    void OnPrePanel() override;
    void DrawDemoPanel() override;
};

inline const char* FitModeName(ImGuiImageFit fit)
{
    switch (fit)
    {
    case ImGuiImageFit::STRETCH:   return "Stretch";
    case ImGuiImageFit::CONTAIN:   return "Contain";
    case ImGuiImageFit::COVER:     return "Cover";
    case ImGuiImageFit::CUSTOM_UV: return "Custom UV";
    default:                       return "Unknown";
    }
}

struct MemoryImage
{
    std::string name;
    std::string key;
    std::vector<uint8_t> pixels;
    int width = 0;
    int height = 0;
    int channels = 4;
};

inline void ImageViewerDemo::DrawSelectedDemo()
{
    static std::array<char, 260> path_buf = {};
    static ImVec2 preview_size = ImVec2(0.0f, 0.0f);
    static ImGuiImageConfig cfg;

    static bool show_image = false;
    static bool loadToMemory = false;

    static std::vector<MemoryImage> memoryImages;
    static int selectedMemoryImage = -1;

    ImGui::TextWrapped(
        "Displays an image from a file path, or decodes it into memory first "
        "and displays it using DrawTextureFromMemory()."
    );

    if (ImGui::CollapsingHeader("Cache Details"))
    {
		auto& cachedTextures = ImGui::GetCachedTextures();
        ImGui::Text("Cached textures: %d", ImGui::GetCachedTextureCount());
        if (!cachedTextures.empty())
        {
            ImGui::BeginChild("cache_list_child", ImVec2(0, 100), true);
            for (const auto& [key, texture] : cachedTextures)
            {
                ImGui::Text("%s: ID=%u, Size=%dx%d",
                    key.c_str(), texture.id, texture.width, texture.height);
            }
            ImGui::EndChild();
		}

        ImGui::Separator();
    }

    ImGui::InputText("Image Path", path_buf.data(), path_buf.size());

    ImGui::TextWrapped("If size is set to 0,0, the base image size is used instead.");
    ImGui::DragFloat2("Image Size", &preview_size.x, 1.0f, 0.0f, 2048.0f, "%.1f");

    ImGui::Checkbox("Load To Memory", &loadToMemory);

    if (ImGui::Button("Load Image"))
    {
        if (path_buf[0] != '\0')
        {
            show_image = true;

            if (loadToMemory)
            {
                int width = 0;
                int height = 0;
                int channels = 0;

                stbi_uc* data = stbi_load(
                    path_buf.data(),
                    &width,
                    &height,
                    &channels,
                    4
                );

                if (data)
                {
                    MemoryImage image;
                    image.name = path_buf.data();
                    image.key = "memory_image_" + std::string(path_buf.data());
					std::cout << "Loaded image: " << image.name << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;
					std::cout << "Key: " << image.key << std::endl;
                    image.width = width;
                    image.height = height;
                    image.channels = 4;

                    const size_t byteCount =
                        static_cast<size_t>(width) *
                        static_cast<size_t>(height) *
                        4;

                    image.pixels.assign(data, data + byteCount);

                    stbi_image_free(data);

                    memoryImages.push_back(std::move(image));
                    selectedMemoryImage = static_cast<int>(memoryImages.size()) - 1;
                }
            }
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear Current"))
    {
        if (loadToMemory && selectedMemoryImage >= 0)
        {
            if (!ImGui::CleanTexture(memoryImages[selectedMemoryImage].key))
            {
				std::cout << "Failed to clean texture for memory image: " << memoryImages[selectedMemoryImage].key << std::endl;
            }
			memoryImages.erase(memoryImages.begin() + selectedMemoryImage);
        }
        else if (path_buf[0] != '\0')
        {
            ImGui::CleanTexture(path_buf.data());
        }

        show_image = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear All Memory Images"))
    {
        for (const MemoryImage& image : memoryImages)
        {
            if (!ImGui::CleanTexture(image.key))
            {
				std::cout << "Failed to clean texture for memory image: " << image.key << std::endl;
            }
        }

        memoryImages.clear();
        selectedMemoryImage = -1;
        show_image = false;
    }

    if (!memoryImages.empty())
    {
        ImGui::SeparatorText("Memory Images");

        const char* preview =
            selectedMemoryImage >= 0
            ? memoryImages[selectedMemoryImage].name.c_str()
            : "<none>";

        if (ImGui::BeginCombo("Selected Memory Image", preview))
        {
            for (int i = 0; i < static_cast<int>(memoryImages.size()); ++i)
            {
                const bool selected = selectedMemoryImage == i;

                std::string label =
                    memoryImages[i].name +
                    " (" +
                    std::to_string(memoryImages[i].width) +
                    "x" +
                    std::to_string(memoryImages[i].height) +
                    ")";

                if (ImGui::Selectable(label.c_str(), selected))
                    selectedMemoryImage = i;

                if (selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
    }

    if (show_image)
    {
        ImGui::Separator();

        bool image_loaded = false;

        if (loadToMemory)
        {
            if (selectedMemoryImage >= 0 &&
                selectedMemoryImage < static_cast<int>(memoryImages.size()))
            {
                const MemoryImage& image = memoryImages[selectedMemoryImage];

                image_loaded = ImGui::DrawTexture(
                    image.key,
                    image.pixels,
                    image.width,
                    image.height,
                    cfg,
                    preview_size
                );
            }
        }
        else
        {
            image_loaded = ImGui::DrawTexture(
                path_buf.data(),
                cfg,
                preview_size
            );
        }

        if (!image_loaded)
        {
            ImGui::TextColored(
                ImVec4(1, 0, 0, 1),
                "Failed to load image. Check the path and try again."
            );
        }
    }
}

inline void ImageViewerDemo::OnPrePanel()
{
    ImGui::SetNextWindowSize({ 760, 680 }, ImGuiCond_Appearing);
}

inline void ImageViewerDemo::DrawDemoPanel()
{
    static ImGuiImageConfig cfg;
    static std::array<char, 260> path_buf = {};
    static ImVec2 image_size = ImVec2(256.0f, 256.0f);
    static bool show_preview_info = true;
    static bool auto_draw = true;

    ImGui::TextUnformatted("Image Viewer Playground");
    ImGui::Separator();

    ImGui::BeginChild(
        "image_preview_child",
        ImVec2(0, 0),
        ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY
    );

    {
        const bool has_path = path_buf[0] != '\0';

        if (!has_path)
        {
            ImGui::TextDisabled("Enter an image path below to preview it here.");
        }
        else if (auto_draw)
        {
            ImGui::DrawTexture(path_buf.data(), cfg, image_size);
        }
        else
        {
            ImGui::TextDisabled("Auto draw is disabled. Enable it below to preview the image.");
        }

        if (show_preview_info)
        {
            ImGui::Separator();
            ImGui::Text("Path: %s", has_path ? path_buf.data() : "<empty>");
            ImGui::Text("Requested size: %.1f x %.1f", image_size.x, image_size.y);
            ImGui::Text("Fit mode: %s", FitModeName(cfg.fit));
            ImGui::Text("Debug text: %s", cfg.debug ? "enabled" : "disabled");
        }
    }

    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::SeparatorText("Settings");

    if (ImGui::BeginTable("image_cfg_table", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 170.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Image Path");

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##image_path", path_buf.data(), path_buf.size());

        DrawHelpTooltip(
            "Path is cached internally. The first draw loads the image and uploads it to OpenGL. "
            "Later draws reuse the cached texture."
        );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Draw");

        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("Auto draw preview", &auto_draw);

        ImGui::SameLine();

        if (ImGui::SmallButton("Clean this texture"))
        {
            if (path_buf[0] != '\0')
                ImGui::CleanTexture(path_buf.data());
        }

        ImGui::SameLine();

        if (ImGui::SmallButton("Clean all"))
            ImGui::CleanAllTextures();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Size (W,H)");

        ImGui::TableSetColumnIndex(1);
        ImGui::DragFloat2("##image_size", &image_size.x, 1.0f, 0.0f, 4096.0f, "%.1f");

        DrawHelpTooltip("Use 0 for either axis to use the texture's original width or height.");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Fit Mode");

        ImGui::TableSetColumnIndex(1);

        {
            int fit = static_cast<int>(cfg.fit);
            const char* fit_names[] = {
                "Stretch",
                "Contain",
                "Cover",
                "Custom UV"
            };

            if (ImGui::BeginCombo("##fit_mode", fit_names[fit]))
            {
                for (int i = 0; i < IM_ARRAYSIZE(fit_names); ++i)
                {
                    const bool selected = fit == i;

                    if (ImGui::Selectable(fit_names[i], selected))
                    {
                        fit = i;
                        cfg.fit = static_cast<ImGuiImageFit>(i);
                    }

                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }
        }

        DrawHelpTooltip(
            "Stretch distorts to the requested size. "
            "Contain preserves aspect ratio inside the box. "
            "Cover crops while filling the box. "
            "Custom UV uses the UV values below."
        );

        const bool use_custom_uv = cfg.fit == ImGuiImageFit::CUSTOM_UV;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("UV0");

        ImGui::TableSetColumnIndex(1);

        if (!use_custom_uv)
            ImGui::BeginDisabled();

        ImGui::DragFloat2("##uv0", &cfg.uv0.x, 0.005f, -2.0f, 2.0f, "%.3f");

        if (!use_custom_uv)
            ImGui::EndDisabled();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("UV1");

        ImGui::TableSetColumnIndex(1);

        if (!use_custom_uv)
            ImGui::BeginDisabled();

        ImGui::DragFloat2("##uv1", &cfg.uv1.x, 0.005f, -2.0f, 2.0f, "%.3f");

        if (!use_custom_uv)
            ImGui::EndDisabled();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("UV Presets");

        ImGui::TableSetColumnIndex(1);

        if (!use_custom_uv)
            ImGui::BeginDisabled();

        if (ImGui::SmallButton("Full"))
        {
            cfg.uv0 = ImVec2(0.0f, 0.0f);
            cfg.uv1 = ImVec2(1.0f, 1.0f);
        }

        ImGui::SameLine();

        if (ImGui::SmallButton("Center crop UV"))
        {
            cfg.uv0 = ImVec2(0.25f, 0.25f);
            cfg.uv1 = ImVec2(0.75f, 0.75f);
        }

        ImGui::SameLine();

        if (ImGui::SmallButton("Flip Y"))
        {
            cfg.uv0 = ImVec2(0.0f, 1.0f);
            cfg.uv1 = ImVec2(1.0f, 0.0f);
        }

        if (!use_custom_uv)
            ImGui::EndDisabled();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Tint Color");

        ImGui::TableSetColumnIndex(1);
        ImGui::ColorEdit4("##tint_col", &cfg.tint_col.x, ImGuiColorEditFlags_AlphaBar);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Background Color");

        ImGui::TableSetColumnIndex(1);
        ImGui::ColorEdit4("##bg_col", &cfg.bg_col.x, ImGuiColorEditFlags_AlphaBar);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Border Color");

        ImGui::TableSetColumnIndex(1);
        ImGui::ColorEdit4("##border_col", &cfg.border_col.x, ImGuiColorEditFlags_AlphaBar);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Border Thickness");

        ImGui::TableSetColumnIndex(1);
        ImGui::SliderFloat("##border_thickness", &cfg.border_thickness, 0.0f, 16.0f, "%.1f");

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Debug Text");

        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("Show DrawTexture debug output", &cfg.debug);

        DrawHelpTooltip(
            "Shows path, OpenGL texture ID, original texture size, draw size, and UVs below the image."
        );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Panel Info");

        ImGui::TableSetColumnIndex(1);
        ImGui::Checkbox("Show preview info", &show_preview_info);

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Reset");

        ImGui::TableSetColumnIndex(1);

        if (ImGui::SmallButton("Reset config"))
        {
            cfg = ImGuiImageConfig{};
            image_size = ImVec2(256.0f, 256.0f);
            show_preview_info = true;
            auto_draw = true;
        }

        ImGui::SameLine();

        if (ImGui::SmallButton("Clear path"))
            path_buf[0] = '\0';

        ImGui::EndTable();
    }
}