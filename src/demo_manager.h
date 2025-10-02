#pragma 
#include <vector>
#include <memory>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <misc/freetype/imgui_freetype.h>
#include <GLFW/glfw3.h>
#include "demo_module.h"
#include "fonts/sourceSans3_regular.h"

static inline bool s_use_freetype = false;
static inline bool s_load_color = false;

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

		{
			ImFontConfig cfg;
			cfg.FontDataOwnedByAtlas = false; // data lives in your compiled binary
			cfg.OversampleH = cfg.OversampleV = 1;
			cfg.GlyphExtraAdvanceX = 1.0f;
			cfg.GlyphOffset.x = 1.0f;
			cfg.PixelSnapH = true;
			cfg.FontLoaderFlags |= ImGuiFreeTypeBuilderFlags_LoadColor | ImGuiFreeTypeBuilderFlags_LightHinting;
			io.Fonts->SetFontLoader(ImGuiFreeType::GetFontLoader());
			io.Fonts->TexMinHeight = 4096; io.Fonts->TexMinWidth = 4096;
			ImFont* ui = io.Fonts->AddFontFromMemoryTTF(
				(void*)SourceSans3_Regular_data,
				(int)SourceSans3_Regular_size,
				18.0f, &cfg);
		}

		{
			static ImWchar ranges[] = { 0x1, 0x1FFFF, 0 };
			static ImFontConfig cfg;
			cfg.OversampleH = cfg.OversampleV = 1;
			cfg.MergeMode = true;
			cfg.FontLoaderFlags |= ImGuiFreeTypeLoaderFlags_LoadColor;
			io.Fonts->SetFontLoader(ImGuiFreeType::GetFontLoader());
			io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguiemj.ttf", 16.0f, &cfg, ranges);
		}

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