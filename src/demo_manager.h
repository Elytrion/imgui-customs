#pragma 
#include <vector>
#include <memory>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include "demo_module.h"

class DemoManager
{
public:
	static void Init(GLFWwindow* window)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460");

		InitModules();
	}

	static void NewFrame();

	static void Draw();

	static void StartRender()
	{
		ImGui::Render();
	}

	static void EndRender()
	{
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	static void Cleanup()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	static void RegisterDemoModule(std::shared_ptr<DemoModule> module)
	{
		demo_modules.push_back(module);
	}

	static const std::vector<std::shared_ptr<DemoModule>>& GetDemoModules() { return demo_modules; }

private:
	static void DrawDockspaceWindow();
	static void DrawCustomImguiDemo();

	static constexpr const char* dockspace_window_name{ "##Dockspace Window" };
	static constexpr const char* main_dockspace_id_name{ "MainDockspace" };
	static constexpr const char* custom_demo_window_name{ "ImGui Customs Demo" };

	static void InitModules();
	static inline std::vector<std::shared_ptr<DemoModule>> demo_modules;
};