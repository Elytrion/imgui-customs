#include "demo_manager.h"
// custom imgui headers
#include "dockEnforcer/imguiDockEnforcer.h"

void DemoManager::DrawDockspaceWindow()
{
    ImGuiViewport* vp = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoBackground;
    // not sure why this is needed, but it is, dock enforcer seems not like the module structure
    // you should be able to use dock enforcer as is
	ImGui::DockEnforcerBeginLock(dockspace_window_name); 
    if (ImGui::Begin(dockspace_window_name, nullptr, flags))
    {
		ImGui::DockEnforcerEndLock();
        ImGuiID dockspace_id = ImGui::GetID(main_dockspace_id_name);
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        static bool dock_built = false;
        if (!dock_built)
        {
            dock_built = true;
            // Check if the dockspace tree already exists
            if (ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id))
            { 
                const bool oldlayout = (node->IsSplitNode() || node->Windows.Size > 0);
                if (!oldlayout) // build default layout
                {
                    ImGui::DockBuilderRemoveNode(dockspace_id);
                    ImGui::DockBuilderAddNode(dockspace_id,
                        ImGuiDockNodeFlags_DockSpace);
                    ImGui::DockBuilderSetNodeSize(dockspace_id, vp->Size);

                    ImGuiID dock_right = ImGui::DockBuilderSplitNode(
                        dockspace_id, ImGuiDir_Right, 0.3f,
                        nullptr, &dockspace_id);

                    ImGui::DockBuilderDockWindow(custom_demo_window_name, dock_right);
                    ImGui::DockBuilderFinish(dockspace_id);
                }
            }
        }

        ImGui::End();
    }
}

void DemoManager::DrawCustomImguiDemo()
{
	bool drawn = ImGui::Begin(custom_demo_window_name, nullptr);
    if (!drawn) { ImGui::End(); return; }
    ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 171, 120, 255));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 194, 133, 255));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 185, 130, 255));
    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::SeparatorText("USER GUIDE:");
        DrawPlaceholderText();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 3.0f);
    }
	ImGui::PopStyleColor(3);

    for (auto& module : demo_modules)
    {
		module->DrawSelector();
	}

	ImGui::End();

    for (auto& module : demo_modules)
    {
        module->BackgroundUpdate();
		if (module->has_popout)
            module->DrawPopoutPanel();
	}
}

void DemoManager::NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::DockEnforcerPreNewFrame();

    ImGui::NewFrame();
}

void DemoManager::Draw()
{
    DrawDockspaceWindow();

	DrawCustomImguiDemo();
}