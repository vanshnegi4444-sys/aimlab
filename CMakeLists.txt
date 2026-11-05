cmake_minimum_required(VERSION 3.24)

# WHY 3.24: FetchContent behaviour we rely on (GIT_SHALLOW etc.) is stable here.
# The user configures on Windows with:  cmake -B build
# and builds with:                     cmake --build build --config Release
project(valo-aim VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)   # pure ISO C++20, no compiler extensions

# /W4 only on OUR code — Dear ImGui was not written warning-clean for /W4,
# so we leave the imgui target at the default warning level.
if(MSVC)
    add_compile_options(/utf-8) # source files are UTF-8; keeps MSVC from misreading them
endif()

include(FetchContent)

# ---------------------------------------------------------------- GLFW 3.4 --
# WHY FetchContent instead of a submodule: zero setup for the user — the first
# `cmake -B build` downloads and builds GLFW automatically. No installer needed.
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glfw)

# ------------------------------------------------------- Dear ImGui (manual) --
# WHY manual: Dear ImGui ships no CMakeLists.txt on purpose (it is a "copy the
# files" library). So we populate the sources and list the .cpp files ourselves.
# Only the core + the two backends we need; imgui_demo.cpp is skipped to keep
# the build fast (it is ~9k lines of demo widgets we don't use).
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.93.0
    GIT_SHALLOW    TRUE
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src   # so the backend finds gl_loader.h (see below)
)
target_link_libraries(imgui PUBLIC glfw)

# In ImGui v1.90+, imgui_impl_opengl3 uses its own built-in loader (imgui_impl_opengl3_loader.h).
# Our renderer and app use gl_loader.h for our own drawing.

# --------------------------------------------------------------- our program --
add_executable(valo-aim
    src/main.cpp
    src/gl_loader.cpp
    src/input.cpp
    src/trainer.cpp
    src/renderer.cpp
    src/ui.cpp
    src/audio.cpp
    src/scenarios.cpp
    src/routine.cpp
    src/map_mesh.cpp
    src/player.cpp
    src/preaim_course.cpp
)
target_include_directories(valo-aim PRIVATE src)
target_link_libraries(valo-aim PRIVATE glfw imgui)

if(MSVC)
    target_compile_options(valo-aim PRIVATE /W4)
endif()

if(WIN32)
    # opengl32.lib has the OpenGL 1.1 entry points; everything newer comes from
    # our runtime loader (gl_loader.cpp) via glfwGetProcAddress.
    # winmm is for procedural low-latency audio via waveOut.
    target_link_libraries(valo-aim PRIVATE opengl32 winmm)
endif()

# Keep a console window in v1: printf debugging is useful while learning, and
# startup errors (e.g. raw-input registration failing) are visible there.
# (A "windowed subsystem, no console" build is a one-line change: set the
# WIN32_EXECUTABLE property — noted in README as a future tweak.)
