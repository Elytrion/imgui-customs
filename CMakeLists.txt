cmake_minimum_required(VERSION 3.20)
project(imgui_customs LANGUAGES C CXX)

# ---- Configuration ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Recommend static runtime on MSVC for "portable" exe (optional)
if (MSVC)
  add_compile_options(/permissive- /Zc:preprocessor /Zc:__cplusplus)
  # set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# On Windows, prefer bundled GLFW binaries off to avoid UCRT dll hassles
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Dear ImGui demo on by default (toggle)
option(IMGUI_ENABLE_DEMO "Build Dear ImGui demo code" ON)

include(FetchContent)

# ---- Fetch GLFW ----
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.4
)
FetchContent_MakeAvailable(glfw)

# ---- Fetch GLM (header-only) ----
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

# ---- Fetch FreeType ----
FetchContent_Declare(
  freetype
  GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
  GIT_TAG VER-2-14-0
)
# Disable HarfBuzz integration in FreeType
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)

# ---- Create a FreeType options override header ----
set(FT_OVERRIDES_DIR ${CMAKE_CURRENT_BINARY_DIR}/ft_overrides)
file(MAKE_DIRECTORY ${FT_OVERRIDES_DIR})

# This file turns on ClearType-style subpixel rendering.
file(WRITE ${FT_OVERRIDES_DIR}/myftoptions.h
"#pragma once
#define FT_CONFIG_OPTION_SUBPIXEL_RENDERING
/* include default options after our overrides (this file does not #undef) */
#include <freetype/config/ftoption.h>
")
# Tell the FreeType sub-build to use our override header
set(FT_CONFIG_OPTIONS_H "<myftoptions.h>")
FetchContent_MakeAvailable(freetype)  # provides Freetype::Freetype
# Make sure FreeType sees the override header on its include path,
# and that FT_CONFIG_OPTIONS_H is defined for that target.
target_include_directories(freetype PRIVATE "${FT_OVERRIDES_DIR}")
target_compile_definitions(freetype PRIVATE "FT_CONFIG_OPTIONS_H=${FT_CONFIG_OPTIONS_H}")

# ---- Fetch Dear ImGui (no native CMake, so we create a target) ----
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking
)
FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
  FetchContent_Populate(imgui)
  # Create a library target for ImGui + backends
  add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    $<$<BOOL:${IMGUI_ENABLE_DEMO}>:${imgui_SOURCE_DIR}/imgui_demo.cpp>
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    ${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
  )
  target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${imgui_SOURCE_DIR}/misc/freetype      # for imgui_freetype.h
  )
  # Tell backend we use GLAD as loader (so it includes <glad/glad.h>) with Freetype
  target_compile_definitions(imgui PUBLIC
    IMGUI_IMPL_OPENGL_LOADER_GLAD
    IMGUI_ENABLE_FREETYPE
    IMGUI_USE_WCHAR32
  )
  # ImGui needs glfw; FreeType for the font builder
  target_link_libraries(imgui PUBLIC glfw freetype)
endif()

# ---- GLAD (vendored; no FetchContent) ----
add_library(glad STATIC
  external/glad/src/glad.c
)
target_include_directories(glad PUBLIC external/glad/include)

# ---- App target ----
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

add_executable(imgui_customs ${SRC})

target_include_directories(imgui_customs PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (MSVC)
  # Put sources under "src/..." filters that mirror the tree
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src FILES ${SRC})

  # (nice to have) put third-party targets in their own Solution folder
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  set_target_properties(imgui PROPERTIES FOLDER "3rdparty")
  set_target_properties(glad  PROPERTIES FOLDER "3rdparty")
  set_target_properties(glfw  PROPERTIES FOLDER "3rdparty")
endif()

target_link_libraries(imgui_customs PRIVATE
  imgui
  glfw
  glad
  glm::glm
)

# Windows: link necessary system libs
if (WIN32)
  target_link_libraries(imgui_customs PRIVATE opengl32)
endif()

# Enable high-DPI awareness on Windows (optional but nice)
if (MSVC)
  target_link_options(imgui_customs PRIVATE "/MANIFESTUAC:level='asInvoker' uiAccess='false'")
endif()

# ---- Runtime on Windows (copy glfw dll if generated as SHARED) ----
# Not needed if GLFW is static (default on MSVC). If you flip to shared, add a post-build copy step.

# ---- Install (optional) ----
install(TARGETS imgui_customs RUNTIME DESTINATION .)
