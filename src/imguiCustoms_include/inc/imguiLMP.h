#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <functional>
#include <iostream>

// namespace used for local popup modal management, storing state and current window information
namespace _lpm
{
	constexpr ImGuiID kMask = 0x4C0CA11u; // Local Popup Modal mask
	inline ImGuiID makeKey(ImGuiID id) { return id ^ kMask; }   // mask the ID
	// Get a reference to the state storage for the popup modal
	inline bool* state(ImGuiWindow* host, ImGuiID id)
	{
		return host->StateStorage.GetBoolRef(makeKey(id), false);
	}
	static ImGuiWindow* currFrameWindow = nullptr; // current window for the frame before the popup
	static ImGuiWindowFlags currFrameWindowFlags = 0; // flags for the current window for the frame
	// Flags to disable inputs and interactions in the host window when modal is open
	ImGuiWindowFlags disablerFlags =
		ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoResize;
}

namespace ImGui
{
	// Set local modal popup state to open. This popup will be local to the current window.
	inline void OpenLocalPopupModal(const char* str_id)
	{
		ImGuiWindow* host = GetCurrentWindow();
		*_lpm::state(host, host->GetID(str_id)) = true;
	}
	// Manually close a local popup modal with the given ID. Should be called after BeginLocalPopupModal returns true.
	inline void CloseLocalPopupModal(const char* str_id)
	{
		if (!_lpm::currFrameWindow)
		{
			std::cerr << "Error: No current window set for closing local popup modal." << std::endl;
			return;
		}
		ImGuiID id = _lpm::currFrameWindow->GetID(str_id);
		*_lpm::state(_lpm::currFrameWindow, id) = false;
		_lpm::currFrameWindow->WindowClass.DockNodeFlagsOverrideSet &= ~ImGuiDockNodeFlags_NoResize; // lock resize even if docked
		_lpm::currFrameWindow->Flags = _lpm::currFrameWindowFlags; // restore flags
	}
	inline void EndLocalPopupModal()
	{
		ImGui::End();
		ImGui::End();
		_lpm::currFrameWindow = nullptr; // reset current window after ending the popup modal
		_lpm::currFrameWindowFlags = 0;
	}
	// Begin a local popup modal with the given ID. Returns true if the popup is open.
	inline bool BeginLocalPopupModal(const char* str_id,
		bool* p_open = nullptr,								// external toggle for the popup modal
		ImGuiWindowFlags extra_window_flags = 0,			// extra window flags for the popup modal
		float dim_bg_alpha = 0.50f,							// dim background alpha when the popup is open (colour follows ImGuiCol_ModalWindowDimBg)
		const ImVec2& popup_pivot = { 0.5f, 0.5f }) 		// the center pivot of the popup relative to the host window

	{
		ImGuiWindow* host = GetCurrentWindow();
		_lpm::currFrameWindow = host; // store current window for later use
		ImGuiID      id = host->GetID(str_id);
		bool& opened = *_lpm::state(host, id);
		// external toggle
		if (p_open && !*p_open) opened = false;
		if (!opened) return false;
		_lpm::currFrameWindowFlags = host->Flags; // save current flags
		_lpm::currFrameWindow->WindowClass.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoResize; // lock resize even if docked
		host->Flags |= _lpm::disablerFlags;

		ImGui::SetNextWindowPos(host->Pos);
		ImGui::SetNextWindowSize(host->Size);
		ImGui::SetNextWindowViewport(host->ViewportId);

		ImGuiWindowFlags ovl_flags =
			ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNavInputs;
		ImVec4 col = ImGui::GetStyleColorVec4(ImGuiCol_ModalWindowDimBg);
		col.w *= dim_bg_alpha;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, col);
		char bg_name[256]; // "unique" background name for the modal overlay, based on id
		std::snprintf(bg_name, sizeof(bg_name), "##LocalModalOverlay_%08X", id);
		ImGui::Begin(bg_name, nullptr, ovl_flags);
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		ImDrawList* dl = host->DrawList;
		ImVec2      min = host->Pos;
		ImVec2      max = { min.x + host->Size.x, min.y + host->Size.y };
		ImRect host_r(min, max);
		ImVec2 popup_pos = host_r.GetCenter();

		ImGui::SetNextWindowPos(popup_pos, ImGuiCond_Always, popup_pivot);
		ImGui::PushClipRect(min, max, true);                        // stay inside host

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize
			| extra_window_flags;

		char win_name[256]; // "unique" name for the local popup modal window, based on id
		std::snprintf(win_name, sizeof(win_name), "###LocalModal_%08X", id);

		bool visible = ImGui::Begin(win_name, nullptr, flags);
		ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
		if (!visible)
		{
			EndLocalPopupModal();
		}

		return visible;
	}
	// Begin a local popup modal with the given ID. Returns true if the popup is open.
	inline bool BeginLocalPopupModal(const char* str_id, ImGuiWindowFlags extra_window_flags,
		float dim_bg_alpha = 0.50f, const ImVec2& popup_pivot = { 0.5f, 0.5f })
	{
		return BeginLocalPopupModal(str_id, nullptr, extra_window_flags, dim_bg_alpha, popup_pivot);
	}
	// Begin a local popup modal with the given ID. Returns true if the popup is open.
	inline bool BeginLocalPopupModal(const char* str_id, float dim_bg_alpha,
		const ImVec2& popup_pivot = { 0.5f, 0.5f })
	{
		return BeginLocalPopupModal(str_id, nullptr, 0, dim_bg_alpha, popup_pivot);
	}
	// Call this to end the local popup modal, resetting the current window and flags. MUST be called after BeginLocalPopupModal returns true.

}
