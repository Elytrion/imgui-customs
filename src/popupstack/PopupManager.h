#pragma once
#include <imgui.h>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cstdint>

enum class PopupSizeMode : uint8_t
{
	AUTO,       // auto size (default) uses imgui auto sizing
	PIXELS,     // size in pixels
	PERCENT     // size in percent of viewport [0 - 1] (0.5f = 50% of viewport)
};
enum class PopupPosition : uint8_t
{
	CENTERED,       // centered in viewport (default)
	TOP_LEFT,       // top-left corner of viewport
	TOP_RIGHT,      // top-right corner of viewport
	BOTTOM_LEFT,    // bottom-left corner of viewport
	BOTTOM_RIGHT    // bottom-right corner of viewport
};
enum class PopupAnimMode : uint8_t
{
	NONE,           // no animation (default)
	FADE,
	SCALE
}; 
struct PopupAnimConfig
{
	PopupAnimMode               mode{ PopupAnimMode::FADE };   // animation mode
	float                       duration{ 0.1f };             // duration in seconds
	std::function<float(float)> easeFunc{ nullptr };
};

using PopupHandle = const char*; // stable ptr handles
using HandleNodeIt = std::list<std::string>::iterator;

struct PopupConfig
{
	PopupSizeMode    size_mode{ PopupSizeMode::PERCENT };   // how to interpret size
	ImVec2           size{ 0, 0 };                          // size in pixels or percent of viewport [0 - 1]
	PopupPosition    position{ PopupPosition::CENTERED };   // where to position the popup
	float 		     padding{ 5.0f };                     // padding around the popup

	ImGuiWindowFlags flags{
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoMove
	};
	bool             modal{ true };                         // true => modal popup (blocks host window), false => normal popup
	PopupAnimConfig  anim_cfg{};                            // animation configuration
};


struct PopupDef
{
	std::string           title;                    // visible title (before ###)
	std::string           label;                    // "<title>###popup_<id>", used internally, users access via PopupHandle
	PopupConfig           cfg{};                    // configuration
	std::function<void()> draw;                     // user content callback
	bool                  reusable{ false };        // false => destroy def+handle after close
};

struct ActiveItem
{
	PopupHandle handle{};                           // key into store map
	bool        opened_once{ false };               // ImGui::OpenPopup() armed?
	bool        close_requested{ false };           // mark to close this frame
	bool        reusable{ true };                   // false => destroy def+handle after close
	bool        animating{ false };					// currently playing an anim
	float       anim_t{ 0.0f };						// 0 - 1
};

class PopupManager {
public:

	// Reserve a popup definition, optionally open immediately.
	PopupHandle Reserve(
		const std::string& title, const PopupConfig& cfg,
		std::function<void()> draw,
		bool open_immediately = false, bool reusable = true);

	// Convenience function to open a popup immediately, calls Reserve with open_immediately=true, reusable=false
	PopupHandle Open(
		const std::string& title, const PopupConfig& cfg,
		std::function<void()> draw);

	// Change the draw function of a reserved popup
	void ModifyReserved(
		PopupHandle h, const PopupConfig& cfg,
		std::function<void()> draw = nullptr
	);

	// Mark a popup to be drawn this frame, optionally float top of the popup stack (foremost popup)
	void AddToDrawStack(PopupHandle h, bool float_up = true, bool reusable = false);

	// Close a popup by its handle; if nullptr, closes the topmost popup
	void Close(PopupHandle h = nullptr);

	// Delete a reserved popup definition and its handle.
	void DeleteReserved(PopupHandle h);

	// Render all popups in the draw stack.
	void DrawAll();

	// Read a popup definition by its handle.
	const PopupDef* GetDef(PopupHandle h) const
	{
		auto it = m_popupDefs.find(h);
		return (it != m_popupDefs.end()) ? &it->second : nullptr;
	}

	struct PopupAnimState
	{
		bool pushed_alpha = false;
		bool pushed_popup_bg = false;
		bool pushed_modal_bg = false;
		bool pushed_window_bg = false;
	};
private:

	std::list<std::string>						m_popupHandles;     // owns labels; provides stable char* handles
	std::unordered_map<PopupHandle, PopupDef>	m_popupDefs;		// handle -> definition
	std::vector<ActiveItem>						m_drawStack;        // active popups to draw this frame

	uint64_t m_token_counter{ 0 };      // unique token for each popup handle
	bool     m_pending_clean{ false };  // true => clean up handles after next DrawAll()

	int         indexOfActive(PopupHandle h) const;
	std::string makeLabel(const std::string& title);                    // Create a unique label for the popup, e.g. "<title>###popup_<id>"
	void        setupNext(const PopupDef& d);                           // Setup next popup position and viewport based on the PopupDef configuration
	PopupAnimState        updateAnim(ActiveItem& a, const PopupDef& d);
	void        cleanHandles();                                         // Clean up handles that are no longer in use, removing them from the draw stack and definitions
	void        removeHandleNode(PopupHandle h);                        // Remove a handle node from the popup handles list
};

#include "PopupPresets.h"