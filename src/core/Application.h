#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "core/InputManager.h"
#include "core/PersistentConfig.h"
#include "editor/CodeEditor.h"
#include "graphics/FullscreenQuadRenderer.h"
#include "graphics/GLResourceDiagnostics.h"
#include "gui/EditorLayout.h"
#include "shader/ShaderCompilationService.h"

namespace app {

class Application {
public:
    Application() = default;
    ~Application() = default;

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool Run();

private:
    bool Initialize();
    void Shutdown();

    bool InitializeWindow();
    bool InitializeOpenGL();
    bool InitializeImGui();
    bool CreateScenePipeline();
    void LoadPersistentState();
    void SavePersistentState() const;
    static std::string PersistentConfigPath();

    void ProcessInput();
    void UpdateTimers();
    void UpdateState();
    void UpdateUniforms();
    void RenderScene();
    void RenderGui();
    bool ConsumeOpenGLErrors(const char* stage);
    void RequestTabSwitch(int tabIndex);
    void SetLogStatus(const std::string& status, const std::string& text, shader::LogSeverity severity, bool focusLogs);
    bool ValidateResourceSnapshot(const char* stage, const graphics::GLResourceSnapshot& expected, bool focusLogs);
    void ToggleFullscreen();
    void EnterFullscreen();
    void ExitFullscreen();

    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    void OnFramebufferSize(int width, int height);

private:
    GLFWwindow* window_ = nullptr;
    bool glfwInitialized_ = false;
    bool imguiInitialized_ = false;

    int framebufferWidth_ = 1280;
    int framebufferHeight_ = 720;

    gui::EditorLayout layout_;
    gui::EditorLayoutState uiState_;
    editor::CodeEditor codeEditor_;
    core::InputManager inputManager_;
    core::PersistentAppState persistentState_;

    graphics::FullscreenQuadRenderer quadRenderer_;
    shader::ShaderCompilationService shaderCompiler_{quadRenderer_};
    graphics::FullscreenQuadRenderState renderState_{};

    std::string shaderLog_;
    std::string currentShaderSource_;

    std::chrono::steady_clock::time_point previousFrameTime_{};
    float iTime_ = 0.0f;
    float iDeltaTime_ = 0.0f;
    std::uint64_t iFrame_ = 0;

    bool leftMouseDown_ = false;
    float mouseDownX_ = 0.0f;
    float mouseDownY_ = 0.0f;
    bool isFullscreen_ = false;
    bool previousEscDown_ = false;
    int windowedPosX_ = 0;
    int windowedPosY_ = 0;
    int windowedWidth_ = 1280;
    int windowedHeight_ = 720;

    graphics::GLResourceSnapshot activeResourceSnapshot_{};
    bool activeResourceSnapshotValid_ = false;
};

} // namespace app
