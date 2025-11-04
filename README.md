# imgui-customs
A collection of semi-useful custom imgui elements and widgets

## Build (Visual Studio)
1. Clone repo
2. Open the folder in Visual Studio (File → Open → Folder…)
3. Choose a CMake Preset (e.g., *Windows x64 - Debug*) if prompted
4. Build (Ctrl+Shift+B) and Run (F5)

On first configure, CMake auto-fetches:
- [GLFW](https://github.com/glfw/glfw)
- [GLM](https://github.com/g-truc/glm)
- [Dear ImGui](https://github.com/ocornut/imgui)
- `GLAD` is vendored in `external/glad` and requires no extra setup.
