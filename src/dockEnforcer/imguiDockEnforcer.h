#pragma once
#include <unordered_map>
#include <string>
#include <cassert>
#include <imgui.h>
#include <imgui_internal.h>

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

    static bool IsWindowBeingDragged(const char* window_name)
    {
        ImGuiContext* ctx = ImGui::GetCurrentContext();
        if (!ctx || !window_name || !*window_name) return false;
        ImGuiWindow* win = ImGui::FindWindowByName(window_name);
        if (!win) return false;
        ImGuiWindow* moving = ctx->MovingWindow;
        if (!moving) return false;
        ImGuiWindow* root_target = win->RootWindowDockTree ? win->RootWindowDockTree : win->RootWindow;
        ImGuiWindow* root_moving = moving->RootWindowDockTree ? moving->RootWindowDockTree : moving->RootWindow;
        return (root_moving == root_target) && ImGui::IsMouseDown(ImGuiMouseButton_Left);
    }

    static ImGuiID RootIdOf(const ImGuiDockNode* node)
    {
        if (node == nullptr) return 0;
        while (node->ParentNode != nullptr) node = node->ParentNode;
        return node->ID;
    }

    static ImGuiID ResolveRootDockNodeID(const char* window_name)
    {
        if (!window_name || !*window_name) return 0;
        ImGuiWindow* w = ImGui::FindWindowByName(window_name);
        if (!w) return 0;
        ImGuiDockNode* node = w->DockNode ? w->DockNode : w->DockNodeAsHost;
        return node ? RootIdOf(node) : 0;
    }

    static ImGuiDockNode* GetNode(ImGuiID id)
    {
        return ImGui::DockBuilderGetNode(id);
    }

    std::unordered_map<std::string, DockMemory> m_mems;
    bool                                        m_pending_restore{ false }; // Flag to indicate if a restore is pending
	LayoutSnapshot                              m_lastStableSnapshot; // Last stable snapshot for rollback
	LayoutSnapshot                              m_armedSnapshot; // Snapshot to restore if rollback is needed
    IniAutosaveGuard                            m_ini_guard;
};

inline void DockEnforcer::DockEnforcerPreNewFrame()
{
    if (m_pending_restore)
    {
        ImGui::LoadIniSettingsFromMemory(m_armedSnapshot.c_str(), m_armedSnapshot.size());
        ImGuiIO& io = ImGui::GetIO();
        if (io.IniFilename && *io.IniFilename)
            ImGui::SaveIniSettingsToDisk(io.IniFilename);
        m_pending_restore = false;
    }
}

inline void DockEnforcer::DockEnforcerBeginLock(const char* windowName, const char* targetRootWindowName)
{
    if (!windowName) return;
    auto& mem = m_mems[windowName];

    if (targetRootWindowName && *targetRootWindowName)
        mem.target_root = ResolveRootDockNodeID(targetRootWindowName);

    if (IsWindowBeingDragged(windowName))
    {
        if (!mem.rollback_armed)
        {
            size_t sz = 0;
            const char* buf = ImGui::SaveIniSettingsToMemory(&sz);
            m_armedSnapshot.assign(buf, sz);
            m_ini_guard.begin();   // pause autosave while dragging
            mem.rollback_armed = true;
        }
        mem.drop_debounce = 0;
    }
    else if (mem.rollback_armed && mem.drop_debounce == 0)
    {
        // Drag ended this frame -> schedule debounce
        constexpr int debounce_frames = 2; // 2 frames debounce, prevents dockbuilder from interfering with layouts
        mem.drop_debounce = debounce_frames;
    }
}

inline void DockEnforcer::DockEnforcerEndLock()
{
    ImGuiWindow* w = ImGui::GetCurrentWindow();
    if (!w) return;
    auto it = m_mems.find(w->Name);
    if (it == m_mems.end()) return;
    auto& mem = it->second;

    if (IsWindowBeingDragged(w->Name))
        return; // still moving, wait

    if (!mem.rollback_armed)
        return; // nothing armed for this window

    // Consume 1-frame debounce after drop
    if (mem.drop_debounce > 0) {
        --mem.drop_debounce;
        if (mem.drop_debounce > 0) return;
    }

    const bool docked = (w->DockNode && w->DockIsActive);
    const ImGuiID current_root = docked ? RootIdOf(w->DockNode ? w->DockNode
        : w->DockNodeAsHost)
        : 0;
    // Policy: no floating; if target_root is set, must stay in it
    bool ok = docked && (mem.target_root == 0 || current_root == mem.target_root);

    if (!ok) {
        // Defer restore to BEFORE the next NewFrame to avoid fighting layouts this frame
        m_pending_restore = !m_armedSnapshot.empty();
    }
    else {
        // Commit: resume autosave and persist current ini
        m_ini_guard.end_and_optionally_persist(true);
    }
    mem.rollback_armed = false;
}

static DockEnforcer g_dock_enforcer; // Global instance of the enforcer

// ImGui namespace access for convenience
namespace ImGui
{
    // you can use local instances if you want to manage multiple layouts,
    // but you must call their PreNewFrame() before ImGui::NewFrame()

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