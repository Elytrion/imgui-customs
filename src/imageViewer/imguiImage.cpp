#include "imguiImage.h"

#include <filesystem>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include <stb_image.h>

#ifndef GLAD_GL_H_
#include "glad/glad.h"
#endif

#include <imgui_internal.h>

namespace ImGuiImageInternal
{
    class TextureCache
    {
    public:

        static TextureCache& GetInstance()
        {
            static TextureCache instance;
            return instance;
        }

        ~TextureCache()
        {
            Clear();
        }

        int GetTextureCount() const
        {
            return static_cast<int>(m_Textures.size());
		}

        ImGuiTexture* GetFromPath(const std::string& path)
        {
            std::string key = path;

            auto it = m_Textures.find(key);
            if (it != m_Textures.end())
                return &it->second;

            ImGuiTexture texture;
			std::string fullPath = std::filesystem::absolute(path).string();
            if (!LoadTextureFromFile(fullPath.c_str(), texture))
                return nullptr;

            auto [insertedIt, success] = m_Textures.emplace(key, texture);
            return &insertedIt->second;
        }

        ImGuiTexture* GetFromPixels(
            const std::string& key,
            const uint8_t* pixels,
            int width,
            int height
        )
        {
            auto it = m_Textures.find(key);
            if (it != m_Textures.end())
                return &it->second;

            if (!pixels || width <= 0 || height <= 0)
                return nullptr;

            ImGuiTexture texture;

            GLuint id = 0;
            glCreateTextures(GL_TEXTURE_2D, 1, &id);

            glTextureStorage2D(id, 1, GL_RGBA8, width, height);
            glTextureSubImage2D(
                id,
                0,
                0,
                0,
                width,
                height,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                pixels
            );

            glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            texture.id = id;
            texture.width = width;
            texture.height = height;

            auto [insertedIt, success] = m_Textures.emplace(key, texture);
            return &insertedIt->second;
        }

        void Clear()
        {
            for (auto& [path, texture] : m_Textures)
            {
                if (texture.id != 0)
                {
                    glDeleteTextures(1, &texture.id);
                    texture.id = 0;
                }
            }

            m_Textures.clear();
        }

        bool RemoveTextureFromCache(const std::string& key)
        {
            auto it = m_Textures.find(key);
            if (it != m_Textures.end())
            {
                if (it->second.id != 0)
                {
                    glDeleteTextures(1, &it->second.id);
                }
                m_Textures.erase(it);
                return true;
            }
            return false;
        }

        const std::unordered_map<std::string, ImGuiTexture>& GetCachedTextures() const
        {
            return m_Textures;
		}

    private:
        static bool LoadTextureFromFile(const char* path, ImGuiTexture& out)
        {
            int width = 0;
            int height = 0;
            int channels = 0;

            stbi_uc* pixels = stbi_load(path, &width, &height, &channels, 4);
            if (!pixels)
                return false;

            GLuint id = 0;
            glCreateTextures(GL_TEXTURE_2D, 1, &id);

            glTextureStorage2D(id, 1, GL_RGBA8, width, height);
            glTextureSubImage2D(
                id,
                0,
                0,
                0,
                width,
                height,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                pixels
            );

            glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            stbi_image_free(pixels);

            out.id = id;
            out.width = width;
            out.height = height;

            return true;
        }

        std::unordered_map<std::string, ImGuiTexture> m_Textures;
    };
}

namespace ImGui
{
    const std::unordered_map<std::string, ImGuiTexture>& GetCachedTextures()
    {
		return ImGuiImageInternal::TextureCache::GetInstance().GetCachedTextures();
    }

    int GetCachedTextureCount()
    {
        auto& cache = ImGuiImageInternal::TextureCache::GetInstance();
        return cache.GetTextureCount();
	}

    bool DrawTexture(const std::string& path, ImGuiImageConfig cfg, ImVec2 size)
    {
        auto& cache = ImGuiImageInternal::TextureCache::GetInstance();
        ImGuiTexture* texture = cache.GetFromPath(path);

        if (!texture || texture->id == 0)
            return false;

        ImVec2 originalSize(
            static_cast<float>(texture->width),
            static_cast<float>(texture->height)
        );

        if (size.x <= 0.0f)
            size.x = originalSize.x;
        if (size.y <= 0.0f)
            size.y = originalSize.y;

        ImVec2 uv0 = cfg.uv0;
        ImVec2 uv1 = cfg.uv1;
        ImVec2 drawSize = size;

        const float imageAspect = originalSize.x / originalSize.y;
        const float boxAspect = size.x / size.y;

        switch (cfg.fit)
        {
        case ImGuiImageFit::CONTAIN:
        {
            if (boxAspect > imageAspect)
                drawSize.x = size.y * imageAspect;
            else
                drawSize.y = size.x / imageAspect;
        }
        break;
        case ImGuiImageFit::COVER:
        {
            if (boxAspect > imageAspect)
            {
                float visibleHeight = imageAspect / boxAspect;
                float crop = (1.0f - visibleHeight) * 0.5f;

                uv0.y = crop;
                uv1.y = 1.0f - crop;
            }
            else
            {
                float visibleWidth = boxAspect / imageAspect;
                float crop = (1.0f - visibleWidth) * 0.5f;

                uv0.x = crop;
                uv1.x = 1.0f - crop;
            }
        }
        break;
        case ImGuiImageFit::CUSTOM_UV:
        {
            uv0 = cfg.uv0;
            uv1 = cfg.uv1;
        }
        break;
		case ImGuiImageFit::STRETCH:
        default:
            break;
        }

		float old_borderSize = ImGui::GetStyle().ImageBorderSize;
		ImGui::GetStyle().ImageBorderSize = cfg.border_thickness;
        ImGui::PushStyleColor(ImGuiCol_Border, cfg.border_col);

        ImGui::ImageWithBg(
            (ImTextureID)(intptr_t)texture->id,
            drawSize,
            uv0,
            uv1,
            cfg.bg_col,
            cfg.tint_col
        );

		ImGui::PopStyleColor();
		ImGui::GetStyle().ImageBorderSize = old_borderSize;

        if (cfg.debug)
        {
            ImGui::Text("Path: %s", path.c_str());
            ImGui::Text("Texture ID: %u", texture->id);
            ImGui::Text("Original: %d x %d", texture->width, texture->height);
            ImGui::Text("Draw size: %.1f x %.1f", drawSize.x, drawSize.y);
            ImGui::Text("UV0: %.3f, %.3f", uv0.x, uv0.y);
            ImGui::Text("UV1: %.3f, %.3f", uv1.x, uv1.y);
        }

        return true;
    }

    bool DrawTexture(
        const std::string& key, const std::vector<uint8_t>& pixels, int width, int height,
        ImGuiImageConfig cfg, ImVec2 size)
    {
        auto& cache = ImGuiImageInternal::TextureCache::GetInstance();
        ImGuiTexture* texture =
            cache.GetFromPixels(key, pixels.data(), width, height);

        if (!texture || texture->id == 0)
            return false;

        if (size.x <= 0.0f)
            size.x = static_cast<float>(texture->width);

        if (size.y <= 0.0f)
            size.y = static_cast<float>(texture->height);

        ImGui::Image(
            (ImTextureID)(intptr_t)texture->id,
            size,
            cfg.uv0,
            cfg.uv1,
            cfg.tint_col,
            cfg.border_col
        );

        return true;
    }

    bool CleanTexture(const std::string& id)
    {
        auto& cache = ImGuiImageInternal::TextureCache::GetInstance();
        return cache.RemoveTextureFromCache(id);
    }

    void CleanAllTextures()
    {
        auto& cache = ImGuiImageInternal::TextureCache::GetInstance();
        cache.Clear();
    }
}