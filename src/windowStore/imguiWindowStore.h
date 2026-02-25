#pragma once
#include <imgui.h>
#include <imgui_internal.h>
namespace ImGui
{
	// used to store per-window data in ImGuiStorage with a convenient API
	// think of WindowStore as a constructable accessor to ImGuiStorage scoped to a specific window and base key
    struct WindowStore
    {
        ImGuiStorage* Storage = nullptr;
        ImGuiID       BaseKey = 0;

        WindowStore(ImGuiStorage* storage, ImGuiID base_key)
            : Storage(storage), BaseKey(base_key) {
        }

        bool Has(const char* tag) const { return Storage->GetBool(ValidTag(tag), false); }
        void SetValid(const char* tag, bool v = true) const { Storage->SetBool(ValidTag(tag), v); }

        bool  GetBool(const char* tag, bool  default_value = false) const { return Storage->GetBool(Key(tag), default_value); }
        int   GetInt(const char* tag, int   default_value = 0)     const { return Storage->GetInt(Key(tag), default_value); }
        float GetFloat(const char* tag, float default_value = 0.0f)  const { return Storage->GetFloat(Key(tag), default_value); }

        void SetBool(const char* tag, bool  v) const { Storage->SetBool(Key(tag), v); SetValid(tag); }
        void SetInt(const char* tag, int   v) const { Storage->SetInt(Key(tag), v); SetValid(tag); }
        void SetFloat(const char* tag, float v) const { Storage->SetFloat(Key(tag), v); SetValid(tag); }

        void* GetPtr(const char* tag) const { return Storage->GetVoidPtr(Key(tag)); }
        void  SetPtr(const char* tag, void* v) const { Storage->SetVoidPtr(Key(tag), v); SetValid(tag); }

        ImVec2 GetVec2(const char* tag, const ImVec2& def = ImVec2(0, 0)) const
        {
            const ImGuiID kx = KeyWithSuffix(tag, "##x");
            const ImGuiID ky = KeyWithSuffix(tag, "##y");
            return ImVec2(Storage->GetFloat(kx, def.x), Storage->GetFloat(ky, def.y));
        }

        void SetVec2(const char* tag, const ImVec2& v) const
        {
            const ImGuiID kx = KeyWithSuffix(tag, "##x");
            const ImGuiID ky = KeyWithSuffix(tag, "##y");
            Storage->SetFloat(kx, v.x);
            Storage->SetFloat(ky, v.y);
            SetValid(tag);
        }

        ImVec4 GetVec4(const char* tag, const ImVec4& def = ImVec4(0, 0, 0, 0)) const
        {
            const ImGuiID kx = KeyWithSuffix(tag, "##x");
            const ImGuiID ky = KeyWithSuffix(tag, "##y");
            const ImGuiID kz = KeyWithSuffix(tag, "##z");
            const ImGuiID kw = KeyWithSuffix(tag, "##w");
            return ImVec4(Storage->GetFloat(kx, def.x),
                Storage->GetFloat(ky, def.y),
                Storage->GetFloat(kz, def.z),
                Storage->GetFloat(kw, def.w));
        }

        void SetVec4(const char* tag, const ImVec4& v) const
        {
            const ImGuiID kx = KeyWithSuffix(tag, "##x");
            const ImGuiID ky = KeyWithSuffix(tag, "##y");
            const ImGuiID kz = KeyWithSuffix(tag, "##z");
            const ImGuiID kw = KeyWithSuffix(tag, "##w");
            Storage->SetFloat(kx, v.x);
            Storage->SetFloat(ky, v.y);
            Storage->SetFloat(kz, v.z);
            Storage->SetFloat(kw, v.w);
            SetValid(tag);
        }

    private:
        ImGuiID Key(const char* tag) const
        {
            IM_ASSERT(Storage != nullptr);
            IM_ASSERT(tag != nullptr);
            // Pure hash: does not depend on CurrentWindow or ID stack
            return ImHashStr(tag, 0, BaseKey);
        }

        ImGuiID ValidTag(const char* tag) const
        {
            // Also pure, and derived from the same stable key.
            const ImGuiID k = Key(tag);
            return ImHashStr("##validtagsalt", 0, k);
        }

        ImGuiID KeyWithSuffix(const char* tag, const char* suffix) const
        {
            char buf[256];
            ImFormatString(buf, IM_ARRAYSIZE(buf), "%s%s", tag, suffix);
            return Key(buf);
        }
    };

    // Helper to create a store for the *current window*.
    // Must be called inside a Begin()/End() window.
    inline WindowStore GetWindowStore(ImGuiID base_key)
    {
        return WindowStore(ImGui::GetStateStorage(), base_key);
    }
}