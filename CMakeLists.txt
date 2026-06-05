cmake_minimum_required(VERSION 3.20)

project(GLSLShaderEditor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8 /W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/glfw/CMakeLists.txt")
    message(FATAL_ERROR "Missing GLFW. Vendor it to external/glfw")
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/glad/include/glad/gl.h")
    message(FATAL_ERROR "Missing GLAD. Vendor it to external/glad")
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/imgui/imgui.h")
    message(FATAL_ERROR "Missing Dear ImGui. Vendor it to external/imgui")
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/ImGuiColorTextEdit/TextEditor.h")
    message(FATAL_ERROR "Missing ImGuiColorTextEdit. Vendor it to external/ImGuiColorTextEdit")
endif()

add_subdirectory(external/glfw)

add_library(glad STATIC
    external/glad/src/glad.c
)
target_include_directories(glad PUBLIC
    external/glad/include
)

add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/ImGuiColorTextEdit/TextEditor.cpp
)
target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
    external/ImGuiColorTextEdit
)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(imgui PUBLIC glfw glad)

set(APP_SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/InputManager.cpp
    src/core/PersistentConfig.cpp
    src/graphics/Shader.cpp
    src/graphics/ShaderProgram.cpp
    src/graphics/VertexArray.cpp
    src/graphics/VertexBuffer.cpp
    src/graphics/GLDebug.cpp
    src/graphics/GLResourceDiagnostics.cpp
    src/graphics/FullscreenQuad.cpp
    src/graphics/FullscreenQuadRenderer.cpp
    src/editor/CodeEditor.cpp
    src/io/FileDialogs.cpp
    src/gui/EditorLayout.cpp
    src/shader/ShaderSourcePipeline.cpp
    src/shader/LineRemapper.cpp
    src/shader/ShaderCompilationService.cpp
)

add_executable(GLSLShaderEditor
    ${APP_SOURCES}
)
target_include_directories(GLSLShaderEditor PRIVATE
    include
    src
)
target_link_libraries(GLSLShaderEditor PRIVATE
    glfw
    glad
    imgui
    opengl32
    comdlg32
)

set_target_properties(GLSLShaderEditor PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
