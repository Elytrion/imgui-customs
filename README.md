# imgui-customs
A collection of semi-useful custom imgui elements and widgets that can be included as header only files. Made to be as simple to integrate as possible and easily customisable by users.
This repo currently includes:
- **Dock Enforcer**  
  A simple utility function to force dockable windows to stay docked (snapping back to previous dock anchor if undocked), with additional parameters to force them to stay docked on specified windows.
  
- **Local Modal Popups**  
  Popups that behave like modal popups, but only prevent access to the window it is called on, leaving the rest of the application untouched. Useful for menu blocking.
  
- **Custom Spinner**  
  A spinner following Material Design 3's design with customisable settings, inspired by [this thread post](https://github.com/ocornut/imgui/issues/1901#issuecomment-1951343738). You might be more interested in [this](https://github.com/dalerank/imspinner) for more spinner types.
  
- **Custom Progress Bar**  
  A progress bar following Material Design 3's design with customisable animations and settings, inspired by [this thread post](https://github.com/ocornut/imgui/issues/1901#issuecomment-1951343738).
  
- **Toggle**  
  A simple animated toggle switch with customisable animations and settings, inspired by [this thread post](https://github.com/ocornut/imgui/issues/1537#issuecomment-355569554). For toggles with significantly more customisability, try [this repo](https://github.com/cmdwtf/imgui_toggle).
  
- **Multi-Toggle**  
  ...or Multi-Selectable. Is basically an animated toggle switch that can have more than 2 states, with customisable animations and settings.

- **Tweens**  
  A tween function that behaves very similarly to the one in [Cocos Creator](https://docs.cocos.com/creator/3.8/manual/en/tween/tween-interface.html). Can be used for smooth animations, lerping, and more, all kept inside one simple function called the same way all ImGui functions are called.

- **Easing Functions**  
  A header full of basic easing functions, based on the [Easing Cheat Sheet](https://easings.net/). All widgets with animation or tweens in this repo can be customised with these easing functions, but these functions can also be used individually in your project.

- **Animated Text**  
  Wobble, Shaky and Gradient animated text functions based on [this thread](https://github.com/ocornut/imgui/issues/1286).

- **Custom Text Formats**  
  Various text formatting functions similar to ImGui's TextWrapped or TextLink functions for constraining text within windows.

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

## To-dos
- [x] Create a Demo system to showcase widgets in a user-friendly manner
- [ ] Fully document code and add comments to help with user customisability
- [ ] Add GIFs to better showcase widgets on README document


*Hello, this project is mainly meant for personal use, but feel free to use anything here. If you wish to contribute to this project, feel free to reach out to me!*
