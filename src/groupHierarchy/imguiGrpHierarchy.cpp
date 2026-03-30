#include "imguiGrpHierarchy.h"
#include <stdexcept>
#include <string>
#include <imgui_internal.h>
namespace
{ 
	constexpr char const* HIER_PAYLOAD = "GEH_ENTRY";
}

namespace ImGui
{
	bool CustomCollapsibleHeader(void* idPtr, const char* label, std::function<void()> drawBody, ImGuiTreeNodeFlags flags, bool internalPadding)
	{
		const ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		//Internal padding
		if (internalPadding)ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

		const ImGuiID id = ImGui::GetID(idPtr);
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImDrawListSplitter scopedSplitter;
		const ImGuiStyle& style = ImGui::GetStyle();
		const ImGuiIO& io = ImGui::GetIO();
		//bool* open = ImGui::GetStateStorage()->GetBoolRef(id, (flags & ImGuiTreeNodeFlags_DefaultOpen));
		bool open = TreeNodeUpdateNextOpen(id, flags);

		// Start tracking position
		const ImVec2 startPos = ImGui::GetCursorScreenPos();
		scopedSplitter.Split(drawList, 2); // Split channels for bg and content (we need to draw the content first, but render the bg behind it)
		scopedSplitter.SetCurrentChannel(drawList, 1); // Set to content channel
		// Horizontal layout: arrow on left, draw shifted content
		const ImVec2 cursor = ImGui::GetCursorPos();
		const ImVec2 arrowSize = ImGui::CalcTextSize("v");
		ImGui::SetCursorPos({ cursor.x + arrowSize.x + 4, cursor.y });

		const ImVec2 contentStart = ImGui::GetCursorScreenPos();
		ImGui::BeginGroup(); // Start a group to track content bounds
		if (drawBody)
			drawBody();
		else
			ImGui::Text(label);
		ImGui::EndGroup(); // End group to finalize content bounds
		const ImVec2 contentEnd = ImGui::GetCursorScreenPos();

		// Compute full bounds
		const float totalHeight = contentEnd.y - contentStart.y;
		const float widthToUse = (flags & ImGuiTreeNodeFlags_OpenOnArrow) ? arrowSize.x : ImGui::GetContentRegionAvail().x;
		const ImVec2 fullSize = { widthToUse, totalHeight };

		//Draw arrow
		ImGui::SetCursorScreenPos(startPos);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (totalHeight - arrowSize.y) / 2.f);
		ImGui::Text(open ? "v" : ">");

		// Reset cursor to original line for drawing overlays
		ImGui::SetCursorScreenPos(startPos);

		// Invisible button over entire area (for click + highlight)
		ImGui::InvisibleButton(label, fullSize);
		const bool hovered = ImGui::IsItemHovered();
		const bool clicked = ImGui::IsItemClicked();

		// Open/close 
		scopedSplitter.SetCurrentChannel(drawList, 0); // Set to background channel
		const ImVec2 rectMaxSize{ startPos.x + fullSize.x, startPos.y + fullSize.y };
		if (clicked)
		{
			open = !open;
			window->DC.StateStorage->SetInt(id, open);
			drawList->AddRectFilled(startPos, rectMaxSize, ImGui::GetColorU32(ImGuiCol_HeaderActive), style.FrameRounding);
		}
		// Highlight background
		else if (hovered)
			drawList->AddRectFilled(startPos, rectMaxSize, ImGui::GetColorU32(ImGuiCol_HeaderHovered), style.FrameRounding);
		scopedSplitter.SetCurrentChannel(drawList, 1); // Switch back to content channel

		//RenderArrow(drawList, arrowPos, GetColorU32(ImGuiCol_Text), dir, arrowScale);
		scopedSplitter.Merge(drawList); // Merge channels back together
		// Advance cursor to next line
		ImGui::SetCursorPos({ cursor.x, cursor.y + totalHeight });

		//Internal padding
		if (internalPadding)ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);

		return open;
	}

	GroupEntryHierarchy::DrawerEntry::DrawerEntry(Type t, int n)
		: type(t), id(std::move(n)) {
	}
	bool GroupEntryHierarchy::DrawerEntry::has(DrawerEntry* child) const
	{
		if (type != Type::GROUP)
			return false;
		return std::find(children.begin(), children.end(), child) != children.end();
	}

	int GroupEntryHierarchy::getEntryIndex(DrawerEntry* entry) const
	{
		const auto& entry_list = entry->parent ? entry->parent->children : rootEntries;
		const auto it = std::find(entry_list.begin(), entry_list.end(), entry);
		if (it == entry_list.end())
			return -1;
		return static_cast<int>(it - entry_list.begin());
	}

	// recursively dissolve a group entry, removing it and its children ONLY if it is empty
	void GroupEntryHierarchy::dissolveGroup(DrawerEntry* group)
	{
		if (group->type != Type::GROUP)
			return; // not a group, nothing to dissolve
		if (!group->children.empty())
			return; // nothing to dissolve, group is not empty

		DrawerEntry* parent = group->parent;

		// Remove from either parent->children or rootEntries
		if (parent)
		{
			auto& siblings = parent->children;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), group),
				siblings.end());
		}
		else
		{
			// it was a root-level group
			rootEntries.erase(std::remove(rootEntries.begin(),
				rootEntries.end(),
				group),
				rootEntries.end());
		}

		// Clear its backlink
		group->parent = nullptr;
		group->type = Type::INVALID; // mark as invalid

		if (immediateDissolve) // if immediate dissolve is enabled, remove it from the master entries vector, if not, just leave it as invalid for later cleanup
		{
			// Erase from the master entries vector
			const auto it = std::find_if(entries.begin(), entries.end(),
				[group](auto const& uptr) { return uptr.get() == group; });
			if (it == entries.end())
				throw std::logic_error("dissolveGroup: entry not found");
			entries.erase(it);
		}

		// If we removed the last child from a parent-group, recursively dissolve it
		if (parent && parent->type == Type::GROUP && parent->children.empty())
			dissolveGroup(parent);
	}
	// remove a group entry, dissolving it and moving its children to the parent or root entries
	void GroupEntryHierarchy::removeGroup(DrawerEntry* group)
	{
		if (group->type != Type::GROUP) return;

		// figure out where its children should land
		DrawerEntry* parent = group->parent;
		auto& destList = parent ? parent->children : rootEntries;
		const int spliceIndex = getEntryIndex(group);
		for (auto it = group->children.rbegin(); it != group->children.rend(); ++it) {
			DrawerEntry* child = *it;
			child->parent = parent;
			int idx = spliceIndex;
			if (idx < 0 || idx > destList.size())
				idx = destList.size();
			destList.insert(destList.begin() + idx, child);
		}
		group->children.clear();

		// detach & remove the group itself
		detach(group);    // unlink it from parent or rootEntries
		const auto it = std::find_if(entries.begin(), entries.end(),
			[group](auto const& up) { return up.get() == group; });
		if (it != entries.end())
			entries.erase(it);

		// recursively dissolve the parent group if it is now empty
		if (parent && parent->type == Type::GROUP && parent->children.empty())
			dissolveGroup(parent);
	}
	// remove any and all references to the entry from its parent, and return the old parent
	GroupEntryHierarchy::DrawerEntry* GroupEntryHierarchy::detach(DrawerEntry* entry)
	{
		// returns the detach entry's group if it was in a group, or nullptr if it was a root entry
		if (!entry) return nullptr;
		if (entry->parent)
		{
			auto& siblings = entry->parent->children;
			siblings.erase(std::remove(siblings.begin(), siblings.end(), entry),
				siblings.end());
		}
		else
		{
			const auto it = std::find(rootEntries.begin(), rootEntries.end(), entry);
			if (it != rootEntries.end())
				rootEntries.erase(it); // remove from root entries if it was a root entry
		}
		DrawerEntry* oldParent = entry->parent;
		entry->parent = nullptr;
		return oldParent;
	}

	// create a new group entry, optionally inserting it into an existing entry's children at a specific index
	GroupEntryHierarchy::DrawerEntry* GroupEntryHierarchy::createGroup(int id, DrawerEntry* existing_entry, int index)
	{
		entries.emplace_back(
			std::make_unique<DrawerEntry>(Type::GROUP,
				std::move(id))
		);
		DrawerEntry* newGroup = entries.back().get();
		auto& entry_list = existing_entry ? existing_entry->children : rootEntries;
		if (index < 0 || index >= entry_list.size())
			entry_list.push_back(newGroup);
		else
			entry_list.insert(entry_list.begin() + index, newGroup);
		if (existing_entry)
			newGroup->parent = existing_entry; // set parent if it was added to a group
		else
			newGroup->parent = nullptr; // root-level group has no parent
		return newGroup;
	}
	// MUST be called to clean up invalid entries if immediateDissolve is false
	void GroupEntryHierarchy::cleanEntries()
	{
		// remove all entries that are marked as invalid
		entries.erase(std::remove_if(entries.begin(), entries.end(),
			[](const std::unique_ptr<DrawerEntry>& uptr) { return uptr->type == Type::INVALID; }),
			entries.end());
	}
	// Create a new entry with the given name and data, optionally inserting it into an existing entry's children at a specific index.
	// Returns the created Entry pointer
	GroupEntryHierarchy::DrawerEntry* GroupEntryHierarchy::createEntry(int id, DrawerEntry* existing_entry, int index)
	{
		entries.emplace_back(
			std::make_unique<DrawerEntry>(Type::ENTRY,
				std::move(id))
		);
		DrawerEntry* newEntry = entries.back().get();
		auto& entry_list = existing_entry ? existing_entry->children : rootEntries;
		if (index < 0 || index >= entry_list.size())
			entry_list.push_back(newEntry);
		else
			entry_list.insert(entry_list.begin() + index, newEntry);
		if (existing_entry)
			newEntry->parent = existing_entry; // set parent if it was added to a group
		else
			newEntry->parent = nullptr; // root-level entry has no parent
		return newEntry;
	}
	// Move an entry to a new parent or root level, optionally at a specific index.
	bool GroupEntryHierarchy::moveEntry(DrawerEntry* entry, DrawerEntry* parent, int index)
	{
		if (!entry) return false; // invalid entry
		if (entry == parent) return false; // can't move an entry into itself
		index = std::max(0, index); // ensure index is non-negative

		for (const DrawerEntry* p = parent; p; p = p->parent) // prevents moving a node into its own subtree
			if (p == entry)
				return false;    // can't move an entry into itself or its children

		// Case 1: Moving Entry to Root
		if (!parent)
		{
			DrawerEntry* oldParent = detach(entry);
			if (oldParent)
				dissolveGroup(oldParent); // recursively dissolve old group if it was in one
			// insert into root entries at index, or append if index is invalid
			if (index >= rootEntries.size())
				rootEntries.push_back(entry);
			else
				rootEntries.insert(rootEntries.begin() + index, entry);
			return true;
		}

		// Case 2: Invalid moves into parent
		DrawerEntry* oldParent = entry->parent;
		const int    oldIndex = getEntryIndex(entry);
		const bool   wasRoot = (oldParent == nullptr);
		if (entry->parent == parent && oldIndex == index) return false; // no change, already in the right place
		if (entry->parent == parent && oldIndex != index) 			    // just move to the new index in the same parent
		{

			entry->parent->children.erase(
				std::remove(entry->parent->children.begin(), entry->parent->children.end(), entry),
				entry->parent->children.end()
			);
			if (index >= parent->children.size())
				parent->children.push_back(entry);
			else
				parent->children.insert(parent->children.begin() + index, entry);
			return true;
		}

		// Case 3: Moving Entry into a Parent
		// Case 3A: Parent is a Group
		if (parent->type == Type::GROUP)
		{
			// detach from any existing group
			const DrawerEntry* oldGroup = detach(entry);
			// insert into new group's children at index, or append if invalid index
			if (index >= parent->children.size())
				parent->children.push_back(entry);
			else
				parent->children.insert(parent->children.begin() + index, entry);
			entry->parent = parent; // set new parent
			// if the old group is now empty, dissolve it
			if (oldGroup)
				dissolveGroup(oldParent);
			return true;
		}
		// Case 3B: Parent is an Entry
		else
		{
			// detach from any existing group for both entries

			DrawerEntry* parentsParent = parent->parent; // get the parent of the parent
			DrawerEntry* newGroup = createGroup(parent->id, parentsParent, index);

			const DrawerEntry* oldGroup = detach(entry);
			parentsParent = detach(parent);
			//// create a new group with the entry and parent
			//Entry* newGroup = createGroup("G_" + parent->name, oldParentGroup, index);
			newGroup->children.push_back(parent);
			newGroup->children.push_back(entry);
			entry->parent = newGroup;
			parent->parent = newGroup;
			// if the old group is now empty, dissolve it (oldParentGroup can never be dissolved here as it must either be null or contain the new grp)
			if (oldGroup)
				dissolveGroup(oldParent);
			return true;
		}

	}
	// Remove an entry from the hierarchy, dissolving it if it is a group.
	void GroupEntryHierarchy::removeEntry(DrawerEntry* entry)
	{
		if (entry->type == Type::GROUP)
		{
			dissolveGroup(entry);
			return;
		}
		detach(entry);
		const auto it = std::find_if(entries.begin(), entries.end(),
			[entry](const std::unique_ptr<DrawerEntry>& uptr) { return uptr.get() == entry; });
		if (it != entries.end())
			entries.erase(it);
		else
			throw std::logic_error("removeEntry: entry not found in master list");
	}

	/*
	* DrawHierarchy is a function that draws a GroupEntryHierarchy object using ImGui.
	* It provides all the drag and drop and reordering functionality for the entries and groups.
	* Draws for entries and groups should be provided by the user as lambda functions parameters.
	* Returns true if selected entry has changed.
	* User can also choose return a bool from the draw functions to indicate if the entry was selected. To accommodate selection outside of this widget.
	*/
	bool DrawHierarchy(
		GroupEntryHierarchy& h,
		int& selectedId,
		int& prevSelectedId,
		int& hoveredId,
		int& dragAndDroppedId,
		DroppedPosition& droppedPosition, //0 - before node, 1 - on node, 2 - after node.
		int customNumSelected,
		std::function<bool(GroupEntryHierarchy::DrawerEntry*, bool dragPopup)> drawEntry,
		std::function<bool(GroupEntryHierarchy::DrawerEntry*, bool dragPopup)> drawGroup,
		bool allowDragAndDropRearrange, bool spanFullWidth, float collapsiblePadding,
		float indentStep, float slotHeight, float dragAndDropLineSpace)
	{
		using DrawerEntry = GroupEntryHierarchy::DrawerEntry;
		using Type = GroupEntryHierarchy::Type;
		const ImU32 hoverColor = ImGui::GetColorU32(ImGuiCol_HeaderHovered);
		const ImU32 hoverInBetweenColor = ImGui::GetColorU32(ImVec4(1.000f, 0.827f, 0.600f, 1.0f));
		const ImU32 activeColor = ImGui::GetColorU32(ImGuiCol_HeaderActive);
		const float rounding = ImGui::GetStyle().FrameRounding;
		dragAndDroppedId = -1;
		bool selectionChanged = false;
		int entryDrawOrder = 0;
		// Recursive draw
		if (!drawGroup)
			drawGroup = drawEntry; // if no group draw function is provided, use the entry draw function
		const bool prevImDissolve = h.immediateDissolve;
		h.immediateDissolve = false; // for drag and drop, we don't want to invalidate the group pointers immediately
		bool hoverOverEntry = false;
		std::function<void(DrawerEntry*, int)> drawNode = [&](DrawerEntry* node, int depth)
			{
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImDrawListSplitter scopedSplitter;
				scopedSplitter.Split(dl, 2); // split channels for background and content
				scopedSplitter.SetCurrentChannel(dl, 1); // set to content channel
				const auto& siblings = node->parent ? node->parent->children : h.rootEntries;
				const int  myIndex = h.getEntryIndex(node);
				bool hoveringSlot = false;
				// drag and drop target for the slot before this node
				{
					const ImVec2  p = ImGui::GetCursorScreenPos();
					const ImRect  r(
						{ p.x + (depth * indentStep), p.y - slotHeight },
						{ p.x + ImGui::GetContentRegionAvail().x, p.y - 1.f }
					);
					constexpr int idMask = 0x10;
					const ImGuiID id = ImGui::GetID((void*)(uintptr_t(node) + idMask));
					if (ImGui::BeginDragDropTargetCustom(r, id))
					{
						if (auto pl = ImGui::AcceptDragDropPayload(HIER_PAYLOAD))
						{
							if (allowDragAndDropRearrange) {
								DrawerEntry* src = *(DrawerEntry**)pl->Data;
								h.moveEntry(src, node->parent, myIndex);
							}
							dragAndDroppedId = node->id;
							droppedPosition = DroppedPosition::ABOVE;
							//h._debug_print();
						}
						if (ImGui::IsMouseHoveringRect(r.Min, r.Max))
						{
							hoveringSlot = true;
							dl->AddLine({ r.Min.x, p.y }, { r.Max.x, p.y }, hoverInBetweenColor);
							dl->AddLine({ r.Min.x, p.y + dragAndDropLineSpace }, { r.Max.x, p.y + dragAndDropLineSpace }, hoverInBetweenColor);
						}
						ImGui::EndDragDropTarget();
					}
				}

				if (depth > 0) ImGui::Indent(depth * indentStep);
				ImGui::PushID(node);
				const ImVec2 itemMin = ImGui::GetCursorScreenPos();
				bool open = true;
				bool entrySelected = false;
				ImGui::BeginGroup();
				if (node->type == Type::ENTRY)
				{
					entrySelected = drawEntry(node, false);
				}
				else
				{
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + collapsiblePadding);
					open = CustomCollapsibleHeader(
						(void*)node,
						std::to_string(node->id).c_str(),
						[&]() {
							entrySelected = drawGroup(node, false);
						},
						ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow, false
					);
				}
				ImGui::EndGroup();

				const ImVec2 itemMax = ImGui::GetItemRectMax();
				const float  fullW = (spanFullWidth) ? ImGui::GetContentRegionAvail().x : abs(itemMax.x - itemMin.x);
				const float  fullH = itemMax.y - itemMin.y;

				ImGui::SetCursorScreenPos(itemMin);
				ImGui::InvisibleButton("##GEH_DRAG_SRC", ImVec2(fullW, fullH));
				const bool hovered = ImGui::IsItemHovered();
				const bool clicked = ImGui::IsItemClicked() && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
				const bool released = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
				const bool shift = ImGui::IsKeyDown(ImGuiMod_Shift);
				scopedSplitter.SetCurrentChannel(dl, 0); // switch to background channel

				//Draw active background if selected
				if (selectedId == node->id || entrySelected)
				{
					dl->AddRectFilled(itemMin, { itemMin.x + fullW, itemMax.y }, activeColor, rounding);
				}
				else if (hoveredId == node->id || hovered) {
					dl->AddRectFilled(itemMin, { itemMin.x + fullW, itemMax.y }, hoverColor, rounding);
				}

				//Handle hovering and selections
				if (!hoveringSlot) {

					//Draw hover background if hovered, and reset hoveredId if not
					if (hovered)
					{
						hoverOverEntry = true;
						hoveredId = node->id;
					}

					//If clicked on non-selected, set as selectedId
					if (clicked && (selectedId != node->id && !entrySelected)) {
						prevSelectedId = selectedId;
						selectedId = node->id;
						selectionChanged = true;
					}
					//If mouse is released on selected, set as selectedId (Not on clicked, to accommodate for dragging multiple entries)
					else if (released && hovered && customNumSelected > 1 && !shift && !(selectedId != node->id && !entrySelected)) {
						prevSelectedId = selectedId;
						selectedId = node->id;
						selectionChanged = true;
					}
				}

				// drag into this node
				{
					const ImRect r(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
					constexpr int idMask = 0x2000;
					const ImGuiID id = ImGui::GetID((void*)(uintptr_t(node) + idMask));
					if (ImGui::BeginDragDropTargetCustom(r, id))
					{
						if (auto pl = ImGui::AcceptDragDropPayload(HIER_PAYLOAD))
						{
							dragAndDroppedId = node->id;
							droppedPosition = DroppedPosition::ON;
							if (allowDragAndDropRearrange) {
								DrawerEntry* src = *(DrawerEntry**)pl->Data;
								if (node->type == Type::GROUP)
									h.moveEntry(src, node, (int)node->children.size());
								else
									h.moveEntry(src, node, myIndex);
							}

							//h._debug_print();
						}
						if (ImGui::IsMouseHoveringRect(r.Min, r.Max))
							ImGui::GetWindowDrawList()->AddRectFilled(r.Min, r.Max, hoverColor, rounding);
						ImGui::EndDragDropTarget();
					}
				}

				scopedSplitter.SetCurrentChannel(dl, 1); // switch back to content channel
				scopedSplitter.Merge(dl); // merge channels back to default
				// show popup on drag
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip))
				{
					if (hoveredId != node->id) dl->AddRect(itemMin, { itemMin.x + fullW, itemMax.y }, hoverInBetweenColor, rounding);
					ImGui::SetDragDropPayload(HIER_PAYLOAD, &node, sizeof(node));
					if (node->type == Type::ENTRY)
						drawEntry(node, true);
					else
						drawGroup(node, true);
					ImGui::EndDragDropSource();
				}

				ImGui::PopID();
				if (depth > 0) ImGui::Unindent(depth * indentStep);

				if (open)
				{
					const auto kids = node->children;
					for (auto* c : kids) {
						c->order = entryDrawOrder++;
						drawNode(c, depth + 1);
					}
				}

				// drag and drop target for the slot after this node
				if (myIndex + 1 == (int)siblings.size())
				{
					const ImVec2 p = ImGui::GetCursorScreenPos();
					const ImRect  r(
						{ p.x, p.y - slotHeight },
						{ p.x + ImGui::GetContentRegionAvail().x, p.y - 1.f }
					);
					constexpr int idMask = 0x3000;
					const ImGuiID id = ImGui::GetID((void*)(uintptr_t(node) + idMask));
					if (ImGui::BeginDragDropTargetCustom(r, id))
					{
						if (auto pl = ImGui::AcceptDragDropPayload(HIER_PAYLOAD))
						{
							dragAndDroppedId = node->id;
							droppedPosition = DroppedPosition::BELOW;
							if (allowDragAndDropRearrange) {
								DrawerEntry* src = *(DrawerEntry**)pl->Data;
								h.moveEntry(src, node->parent, myIndex + 1);
							}
							//h._debug_print();
						}
						if (ImGui::IsMouseHoveringRect(r.Min, r.Max))
							ImGui::GetWindowDrawList()->AddLine({ r.Min.x, r.Max.y }, r.Max, hoverInBetweenColor);
						ImGui::EndDragDropTarget();
					}
				}
			};

		// Kick off
		ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4());
		for (auto* root : h.rootEntries) {
			root->order = entryDrawOrder++;
			drawNode(root, 0);
		}
		if (hoverOverEntry == false) {
			hoveredId = -1;
		}
		ImGui::PopStyleColor();

		h.cleanEntries();
		h.immediateDissolve = prevImDissolve; // restore the immediate dissolve setting

		return selectionChanged;
	}
}