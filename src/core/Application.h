#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "graphics/FullscreenQuad.h"
#include "graphics/Shader.h"
#include "graphics/ShaderProgram.h"
#include "gui/EditorLayout.h"

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

    void ProcessInput();
    void UpdateTimers();
    void UpdateState();
    void UpdateUniforms();
    void RenderScene();
    void RenderGui();

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

    graphics::Shader vertexShader_;
    graphics::Shader fragmentShader_;
    graphics::ShaderProgram shaderProgram_;
    graphics::FullscreenQuad fullscreenQuad_;

    std::string shaderLog_;

    std::chrono::steady_clock::time_point previousFrameTime_{};
    float iTime_ = 0.0f;
    float iDeltaTime_ = 0.0f;
    std::uint64_t iFrame_ = 0;
};

} // namespace app
