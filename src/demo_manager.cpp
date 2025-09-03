#include "demo_manager.h"
// custom imgui headers
#include "imguiDockEnforcer.h"

static void DrawDockspaceWindow()
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

    if (ImGui::Begin("##Dockspace Window", nullptr, flags))
    {
        ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::End();
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

    ImGui::Begin("Hello, ImGui");
    ImGui::TextUnformatted("GLFW + GLAD + GLM + ImGui (GL3)");
    ImGui::Checkbox("Show ImGui Demo", &show_base_imgui_demo);
    ImGui::End();

    if (show_base_imgui_demo) ImGui::ShowDemoWindow(&show_base_imgui_demo);
}