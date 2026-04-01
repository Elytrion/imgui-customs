#pragma once
#include "imguiGrpHierarchy.h"
#include "demo_module.h"

class GrpHierarchyDemo : public DemoModule
{
public:
	GrpHierarchyDemo() : DemoModule("Group Hierarchy", "Group Hierarchy Demo Panel") {}
protected:
	void DrawSelectedDemo();
	void OnPrePanel() override
	{

	}
	void DrawDemoPanel() override
	{
		DrawPlaceholderText();
	}
};

struct SceneObject
{
    int id;
    std::string name;
    bool isFolder = false;
};
static bool gInit = false;
static ImGui::GroupEntryHierarchy gHierarchy;
static std::unordered_map<int, SceneObject> gObjects;

static int gSelectedId = -1;
static int gPrevSelectedId = -1;
static int gHoveredId = -1;
static int gDroppedId = -1;
static ImGui::DroppedPosition gDroppedPosition = ImGui::DroppedPosition::ON;

void BuildDemoHierarchy()
{
    if (gInit)
		return;
    gInit = true;
    gObjects[1] = { 1, "Player", false };
    gObjects[2] = { 2, "Environment", true };
    gObjects[3] = { 3, "Camera", false };
    gObjects[4] = { 4, "Props", true };
    gObjects[5] = { 5, "Barrel", false };
    gObjects[6] = { 6, "Lamp", false };

    auto* player = gHierarchy.createEntry(1);
    auto* env = gHierarchy.createGroup(2);
    auto* camera = gHierarchy.createEntry(3);

    auto* props = gHierarchy.createGroup(4, env);
    gHierarchy.createEntry(5, props);
    gHierarchy.createEntry(6, props);
}

void DrawSceneHierarchy()
{
    auto drawRow = [](ImGui::GroupEntryHierarchy::DrawerEntry* node, bool dragPopup) -> bool
        {
            auto it = gObjects.find(node->id);
            if (it == gObjects.end())
            {
                ImGui::TextUnformatted("<missing>");
                return false;
            }

            const SceneObject& obj = it->second;

            if (node->type == ImGui::GroupEntryHierarchy::Type::GROUP)
            {
                ImGui::TextUnformatted((std::string("[Folder] ") + obj.name).c_str());
            }
            else
            {
                ImGui::TextUnformatted(obj.name.c_str());
            }

            return false; // let DrawHierarchy manage selection
        };

    ImGui::DrawHierarchy(
        gHierarchy,
        gSelectedId,
        gPrevSelectedId,
        gHoveredId,
        gDroppedId,
        gDroppedPosition,
        1,              // customNumSelected
        drawRow,        // drawEntry
        drawRow,        // drawGroup
        true,           // allowDragAndDropRearrange
        true            // spanFullWidth
    );
}

void GrpHierarchyDemo::DrawSelectedDemo()
{
    BuildDemoHierarchy();

    DrawSceneHierarchy();
}
