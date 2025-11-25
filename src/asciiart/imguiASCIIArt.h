#pragma once
#include <string>
#include <imgui.h>

namespace ImGui
{
    inline void DrawAsciiArtFrame(const char* ascii, bool center = false)
    {
        static int unique_counter = 0;
        std::string id = "AsciiArt_" + std::to_string(unique_counter++);

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 text_size = ImGui::CalcTextSize(ascii, nullptr, false, -1.0f);
        ImVec2 child_size(
            text_size.x + style.WindowPadding.x * 2.0f,
            text_size.y + style.WindowPadding.y * 2.0f);

        if (center)
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float x_offset = (avail.x - child_size.x) * 0.5f;
            if (x_offset > 0)
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x_offset);
        }

        if (ImGui::BeginChild(id.c_str(), child_size, true, ImGuiWindowFlags_NoScrollbar))
            ImGui::TextUnformatted(ascii);
        ImGui::EndChild();
    }
    inline void DrawAsciiArt(const char* ascii, bool center = false)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 text_size = ImGui::CalcTextSize(ascii, nullptr, false, -1.0f);
        ImVec2 child_size(
            text_size.x + style.WindowPadding.x * 2.0f,
            text_size.y + style.WindowPadding.y * 2.0f);
        if (center)
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float x_offset = (avail.x - child_size.x) * 0.5f;
            if (x_offset > 0)
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x_offset);
        }
        ImGui::TextUnformatted(ascii);
	}

	void DrawASCIICatPeep(bool withFrame = true, bool center = true)
	{
        /*
             |\__/,|   (`\
           _.|o o  |_   ) )
         -(((---(((--------
        */
        static const char* kCatArt =
            R"(    |\__/,|   (`\
  _.|o o  |_   ) )
  -(((---(((--------)";
        if (withFrame)
            DrawAsciiArtFrame(kCatArt,  center);
		else
            DrawAsciiArt(kCatArt, center);
	}

    void DrawASCIIBunnyStare(bool withFrame = true, bool center = true)
    {
        static const char* kBunnyArt =
            R"((\_/)
( 0_0)
 /  \)";
        if (withFrame)
            DrawAsciiArtFrame(kBunnyArt, center);
        else
            DrawAsciiArt(kBunnyArt, center);
    }

}