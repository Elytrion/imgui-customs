#pragma once
#include "imgui.h"
#include <vector>
#include <memory>
#include <functional>

namespace ImGui
{
    /*
     * GroupEntryHierarchy is a class that manages a hierarchy of entries and groups.
     * Each entry can be either a simple entry with data or a group that can contain other entries or groups
     * It allows for creating, removing, and managing the hierarchy of entries and groups.
     */
    struct GroupEntryHierarchy
    {
    public:
        enum class Type : std::uint8_t { ENTRY, GROUP, INVALID };
        struct DrawerEntry
        {
            Type                        type;
            int                         id;
            int                         order{ 0 }; //Order of entry displayed in widget
            DrawerEntry* parent{ nullptr };
            std::vector<DrawerEntry*>   children;

            DrawerEntry(Type t, int n);
            bool has(DrawerEntry* child) const;
        };
        std::vector<std::unique_ptr<DrawerEntry>> entries;    // master list of all entries
        std::vector<DrawerEntry*> rootEntries;                // root-level entries (no parent, use this to display)
        // get the index of an entry in its parent's children or rootEntries
        bool immediateDissolve{ false }; // if true, dissolve empty groups immediately after removing them
        int getEntryIndex(DrawerEntry* entry) const;
    private:
        // recursively dissolve a group entry, removing it and its children ONLY if it is empty
        void dissolveGroup(DrawerEntry* group);
        // remove a group entry, dissolving it and moving its children to the parent or root entries
        void removeGroup(DrawerEntry* group);
        // remove any and all references to the entry from its parent, and return the old parent
        DrawerEntry* detach(DrawerEntry* entry);

    public:
        // create a new group entry, optionally inserting it into an existing entry's children at a specific index
        DrawerEntry* createGroup(int id, DrawerEntry* existing_entry = nullptr, int index = -1);
        // MUST be called to clean up invalid entries if immediateDissolve is false
        void cleanEntries();
        // Create a new entry with the given name and data, optionally inserting it into an existing entry's children at a specific index.
        // Returns the created Entry pointer
        DrawerEntry* createEntry(int id, DrawerEntry* existing_entry = nullptr, int index = -1);
        // Move an entry to a new parent or root level, optionally at a specific index.
        bool moveEntry(DrawerEntry* entry, DrawerEntry* parent = nullptr, int index = -1);
        // Remove an entry from the hierarchy, dissolving it if it is a group.
        void removeEntry(DrawerEntry* entry);
    };

    enum class DroppedPosition {
        ABOVE,
        ON,
        BELOW
    };

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
        std::function<bool(GroupEntryHierarchy::DrawerEntry*, bool dragPopup)> drawGroup = nullptr,
        bool allowDragAndDropRearrange = true, bool spanFullWidth = true, float collapsiblePadding = 0.f,
        float indentStep = 16, float slotHeight = 10, float dragAndDropLineSpace = 2
    );
}