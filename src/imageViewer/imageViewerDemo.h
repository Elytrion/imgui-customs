#pragma once
#include "imguiImage.h"
#include "demo_module.h"
#include <vector>
#include <algorithm>

class ImageViewerDemo : public DemoModule
{
public:
    ImageViewerDemo() : DemoModule("Image Viewer", "Image Viewer Panel") { }
    void OnCleanup() override
    {
        // Clear the texture cache
        ImGui::CleanAllTextures();
	}

protected:
    void DrawSelectedDemo() override;
    void OnPrePanel() override;
    void DrawDemoPanel() override;

	std::string image_path;
	bool show_image = false;
	ImVec2 img_size;
};

inline void ImageViewerDemo::DrawSelectedDemo()
{
    static char buf[256] = "";
    if (ImGui::InputText("Image Path", buf, sizeof(buf)))
    {
		image_path = buf;
    }
	ImGui::InputFloat2("Image Size", (float*)&img_size, "%.1f", ImGuiInputTextFlags_CharsDecimal);
    if (ImGui::Button("Load Image"))
    {
        if (image_path.empty())
			return;

        show_image = true;
    }
    if (show_image)
    {
        ImGui::Separator();
        ImGui::DrawTexture(image_path, img_size);
        if (ImGui::Button("Clear Image"))
        {
            show_image = false;
			ImGui::CleanTexture(image_path);
            image_path.clear();
			buf[0] = '\0';
        }
	}

}

inline void ImageViewerDemo::OnPrePanel()
{
    ImGui::SetNextWindowSize({ 700, 600 }, ImGuiCond_Appearing);
}

inline void ImageViewerDemo::DrawDemoPanel()
{
    DrawPlaceholderText();
}