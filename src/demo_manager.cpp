#include "demo_manager.h"

void DemoManager::Draw()
{
    static bool show_demo = true;
    ImGui::Begin("Hello, ImGui");
    ImGui::TextUnformatted("GLFW + GLAD + GLM + ImGui (GL3)");
    ImGui::Checkbox("Show ImGui Demo", &show_demo);
    ImGui::End();

    if (show_demo) ImGui::ShowDemoWindow(&show_demo);
}