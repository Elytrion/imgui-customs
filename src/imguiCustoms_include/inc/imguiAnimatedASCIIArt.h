#pragma once
#include "imguiASCIIArt.h"
#include <imgui_internal.h>
#include <fstream>
#include <sstream>
#include <vector>

namespace ImGui
{
    struct AsciiAnimation
    {
        std::vector<std::string> frames;
        float frame_duration = 0.1f; // seconds per frame
        bool loop = true;
        bool playing = true;
        int current_frame = 0;
        double last_switch_time = 0.0;
        ImVec2 max_text_size = ImVec2(0.f, 0.f);
        void RecalculateMaxSize()
        {
            max_text_size = ImVec2(0.f, 0.f);
            for (const auto& f : frames)
            {
                ImVec2 sz = ImGui::CalcTextSize(f.c_str(), nullptr, false, -1.0f);
                max_text_size.x = std::max(max_text_size.x, sz.x);
                max_text_size.y = std::max(max_text_size.y, sz.y);
            }
        }
    };
    inline void DrawAsciiAnimation(AsciiAnimation& anim, bool withFrame = true, bool center = true)
    {
        if (anim.frames.empty())
            return;

        // Update time-based frame index
        double now = ImGui::GetTime();
        if (anim.playing && anim.frame_duration > 0.0f)
        {
            if (now - anim.last_switch_time >= anim.frame_duration)
            {
                anim.last_switch_time = now;
                anim.current_frame++;

                if (anim.current_frame >= static_cast<int>(anim.frames.size()))
                {
                    if (anim.loop)
                        anim.current_frame = 0;
                    else
                    {
                        anim.current_frame = static_cast<int>(anim.frames.size()) - 1;
                        anim.playing = false; // stop at last frame
                    }
                }
            }
        }

        anim.current_frame = ImClamp(anim.current_frame, 0, (int)anim.frames.size() - 1);
        const std::string& frame = anim.frames[anim.current_frame];

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 child_size(
            anim.max_text_size.x + style.WindowPadding.x * 2.0f,
            anim.max_text_size.y + style.WindowPadding.y * 2.0f
        );

        if (withFrame)
            DrawAsciiArtFrame(frame.c_str(), center, child_size);
        else
            DrawAsciiArt(frame.c_str(), center);
    }
    inline AsciiAnimation CreateAsciiAnimation(
        const std::vector<std::string>& frames,
		float frame_duration = 0.1f,
		bool loop = true)
    {
        AsciiAnimation anim;
		anim.frames = frames;
		anim.frame_duration = frame_duration;
        anim.loop = loop;
        anim.RecalculateMaxSize();
		return anim;
    }

	// Load an ASCII animation from text stream.
    // The file must follow the format of:
	// Frame 1 
    // end-of-frame
	// Frame 2
	// the "end-of-frame" line is used to separate frames.
	// each frame must have at least one line.
    inline bool ParseAsciiAnimationStream(std::istream& in, AsciiAnimation& outAnim)
    {
        outAnim.frames.clear();
        std::string line;
        std::string current;

        while (std::getline(in, line))
        {
            if (line == "end-of-frame")
            {
                if (!current.empty())
                {
                    if (!current.empty() && current.back() == '\n')
                        current.pop_back();

                    outAnim.frames.push_back(current);
                    current.clear();
                }
            }
            else
            {
                current += line;
                current += '\n';
            }
        }

        if (!current.empty())
        {
            if (current.back() == '\n')
                current.pop_back();
            outAnim.frames.push_back(current);
        }

        outAnim.current_frame = 0;
        outAnim.last_switch_time = ImGui::GetTime();
        outAnim.playing = true;
        outAnim.RecalculateMaxSize();

        return !outAnim.frames.empty();
    }
    inline bool LoadAsciiAnimationFromFile(const char* path, AsciiAnimation& outAnim)
    {
        std::ifstream in(path, std::ios::in);
        if (!in.is_open())
            return false;

        return ParseAsciiAnimationStream(in, outAnim);
    }
    inline bool LoadAsciiAnimationFromMemory(const char* text, AsciiAnimation& outAnim)
    {
        std::istringstream in(text);
        return ParseAsciiAnimationStream(in, outAnim);
    }
}
