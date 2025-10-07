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

    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::Text("\xf0\x9f\x8d\x89 \xf0\x9f\x8d\x8a \xf0\x9f\x8d\x8b");
        ImGui::Text(u8"\U0001F432 \U0000261D \U0001f21A");
		ImGui::SliderFloat("Font Size", &font_size, 1.0f, 100.0f, "%.1f");
        ImGui::SeparatorText("USER GUIDE:");
		ImGui::ShowUserGuide(); //TODO: replace with custom user guide
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 3.0f);
    }
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0], font_size);
    for (auto& module : demo_modules)
    {
		module->DrawSelector();
	}
    ImGui::PopFont();
	ImGui::End();

    for (auto& module : demo_modules)
    {
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