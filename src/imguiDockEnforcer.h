#pragma once
#include <imgui.h>
#include <unordered_map>
#include <string>
#include <cassert>

class DockEnforcer
{
public:
	void DockEnforcerPreNewFrame();  // MUST be called BEFORE ImGui::NewFrame() to load dock layouts w/o issues
    void DockEnforcerBeginLock(const char* windowToEnforce, const char* specifiedDockWindow = nullptr);
	void DockEnforcerEndLock();
private:
    struct DockMemory
    {
        ImGuiID target_root     = 0;      
        bool    rollback_armed  = false; 
        int     drop_debounce   = 0;      
        ImVec2  last_size       = ImVec2(0, 0);
    };
	using LayoutSnapshot = std::string;
    // Pause autosave during drag to avoid writing mid-drag layouts to disk.
    struct IniAutosaveGuard
    {
        float prev_rate = 0.0f;
        bool  armed = false;
        void begin() {
            if (armed) return;
            ImGuiIO& io = ImGui::GetIO();
            prev_rate = io.IniSavingRate;     // default ~5 seconds
            io.IniSavingRate = FLT_MAX;       // "almost never" autosave during drag
            armed = true;
        }
        void end_and_optionally_persist(bool persist)
        {
            if (!armed) return;
            ImGuiIO& io = ImGui::GetIO();
            io.IniSavingRate = prev_rate;
            if (persist && io.IniFilename && *io.IniFilename)
                ImGui::SaveIniSettingsToDisk(io.IniFilename);
            armed = false;
        }
    };

    std::unordered_map<std::string, DockMemory> m_mems;
    bool                                        m_pending_restore{ false }; // Flag to indicate if a restore is pending
	LayoutSnapshot                              m_lastStableSnapshot; // Last stable snapshot for rollback
	LayoutSnapshot                              m_armedSnapshot; // Snapshot to restore if rollback is needed
    IniAutosaveGuard                            m_ini_guard;
};

static DockEnforcer g_dock_enforcer; // Global instance of the enforcer
// you can use local instances if you want to manage multiple layouts,
// but you must call their PreNewFrame() before ImGui::NewFrame()

namespace ImGui
{
	// Call this before ImGui::NewFrame() to use dock enforcer without issues
    inline void DockEnforcerPreNewFrame()
    {
        g_dock_enforcer.DockEnforcerPreNewFrame();
	}
	// Call this before ImGui::Begin() of the window you want to enforce docking for
	// you must specify the window id to enforce, and optionally the target dock window
    inline void DockEnforcerBeginLock(const char* windowToEnforce, const char* specifiedDockWindow = nullptr)
    {
        g_dock_enforcer.DockEnforcerBeginLock(windowToEnforce, specifiedDockWindow);
    }
	// Call this within the ImGui::Begin()/ImGui::End() scope of the window you want to enforce docking for,
	// preferably as the first call after ImGui::Begin()
    inline void DockEnforcerEndLock()
    {
        g_dock_enforcer.DockEnforcerEndLock();
	}
}