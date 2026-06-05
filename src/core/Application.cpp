#include "core/Application.h"

#include <fstream>
#include <iomanip>
#include <sstream>

#include <glad/gl.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace {

const char* kBuiltInTestShader = R"(void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float t = iTime * 0.75;
    vec3 grad = vec3(uv.x, uv.y, 0.5 + 0.5 * sin(t));
    vec3 waves = 0.25 * cos(t + uv.xyx * 8.0 + vec3(0.0, 2.0, 4.0));
    fragColor = vec4(grad + waves, 1.0);
}
)";

std::string ReadUtf8TextFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string BuildFragmentSource(const std::string& userSource)
{
    const char* injectedPrefix = R"(#version 330 core
out vec4 FragColor;

uniform float iTime;
uniform float iDeltaTime;
uniform int   iFrame;
uniform vec2  iResolution;
uniform vec4  iMouse;
uniform float PARAM1;
uniform float PARAM2;
uniform float PARAM3;

)";

    const char* injectedWrapper = R"(
void main()
{
    vec4 fragColor = vec4(0.0);
    vec2 fragCoord = gl_FragCoord.xy;
    mainImage(fragColor, fragCoord);
    FragColor = fragColor;
}
)";

    return std::string(injectedPrefix) + userSource + injectedWrapper;
}

} // namespace

namespace app {

bool Application::Run()
{
    if (!Initialize()) {
        Shutdown();
        return false;
    }

    while (!glfwWindowShouldClose(window_)) {
        ProcessInput();
        UpdateTimers();
        UpdateState();
        UpdateUniforms();
        RenderScene();
        RenderGui();
        glfwSwapBuffers(window_);
    }

    Shutdown();
    return true;
}

bool Application::Initialize()
{
    if (!InitializeWindow()) {
        return false;
    }

    if (!InitializeOpenGL()) {
        return false;
    }

    if (!CreateScenePipeline()) {
        return false;
    }

    if (!InitializeImGui()) {
        return false;
    }

    uiState_.compileStatus = "Compiled";
    uiState_.logText = "Phase 1 boot successful.";
    previousFrameTime_ = std::chrono::steady_clock::now();
    return true;
}

void Application::Shutdown()
{
    if (imguiInitialized_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        imguiInitialized_ = false;
    }

    if (window_ != nullptr) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    if (glfwInitialized_) {
        glfwTerminate();
        glfwInitialized_ = false;
    }
}

bool Application::InitializeWindow()
{
    if (glfwInit() != GLFW_TRUE) {
        return false;
    }
    glfwInitialized_ = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(framebufferWidth_, framebufferHeight_, "GLSL Shader Editor", nullptr, nullptr);
    if (window_ == nullptr) {
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, &Application::FramebufferSizeCallback);
    return true;
}

bool Application::InitializeOpenGL()
{
    if (gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)) == 0)
    {
        return false;
    }

    glViewport(0, 0, framebufferWidth_, framebufferHeight_);
    return true;
}

bool Application::InitializeImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(window_, true)) {
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 330")) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    imguiInitialized_ = true;
    return true;
}

bool Application::CreateScenePipeline()
{
    std::string compileLog;
    const std::string fragmentSource = BuildFragmentSource(kBuiltInTestShader);
    if (!quadRenderer_.Initialize(fragmentSource, compileLog)) {
        shaderLog_ = compileLog;
        uiState_.compileStatus = "Renderer init failed";
        uiState_.logText = shaderLog_.empty() ? "Fullscreen renderer setup failed." : shaderLog_;
        return false;
    }

    return true;
}

void Application::ProcessInput()
{
    glfwPollEvents();

    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }
}

void Application::UpdateTimers()
{
    const auto now = std::chrono::steady_clock::now();
    const std::chrono::duration<float> delta = now - previousFrameTime_;
    previousFrameTime_ = now;

    iDeltaTime_ = delta.count();

    if (uiState_.isPlaying) {
        iTime_ += iDeltaTime_;
    }

    ++iFrame_;
}

void Application::UpdateState()
{
    if (uiState_.requestTogglePlayback) {
        uiState_.isPlaying = !uiState_.isPlaying;
        uiState_.requestTogglePlayback = false;
    }

    if (uiState_.requestResetTimer) {
        iTime_ = 0.0f;
        iFrame_ = 0;
        uiState_.requestResetTimer = false;
    }

    if (uiState_.requestRecompile) {
        uiState_.requestRecompile = false;

        std::string userFragment = ReadUtf8TextFile("shaders/default.glsl");
        if (userFragment.empty()) {
            userFragment = kBuiltInTestShader;
        }

        const std::string fragmentSource = BuildFragmentSource(userFragment);
        std::string compileLog;
        if (quadRenderer_.RebuildFragmentShader(fragmentSource, compileLog)) {
            uiState_.compileStatus = "Compiled";
            uiState_.logText = "Shader recompiled successfully.";
        } else {
            uiState_.compileStatus = "Compile failed";
            uiState_.logText = compileLog;
            uiState_.currentTab = 2;
        }
    }
}

void Application::UpdateUniforms()
{
}

void Application::RenderScene()
{
    glViewport(0, 0, framebufferWidth_, framebufferHeight_);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    graphics::FullscreenQuadRenderState state;
    state.iTime = iTime_;
    state.iDeltaTime = iDeltaTime_;
    state.iFrame = static_cast<int>(iFrame_);
    state.iResolutionX = static_cast<float>(framebufferWidth_);
    state.iResolutionY = static_cast<float>(framebufferHeight_);
    state.param1 = uiState_.param1;
    state.param2 = uiState_.param2;
    state.param3 = uiState_.param3;

    quadRenderer_.Render(state);
    ConsumeOpenGLErrors("RenderScene");
}

void Application::RenderGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const float fps = (iDeltaTime_ > 0.0f) ? (1.0f / iDeltaTime_) : 0.0f;
    layout_.Render(uiState_, iTime_, fps, framebufferWidth_, framebufferHeight_, iFrame_);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ConsumeOpenGLErrors("RenderGui");
}

void Application::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    auto* self = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (self != nullptr) {
        self->OnFramebufferSize(width, height);
    }
}

void Application::OnFramebufferSize(int width, int height)
{
    framebufferWidth_ = width;
    framebufferHeight_ = height;
}

bool Application::ConsumeOpenGLErrors(const char* stage)
{
    GLenum error = glGetError();
    if (error == GL_NO_ERROR) {
        return true;
    }

    std::ostringstream oss;
    oss << stage << " OpenGL error(s):";
    while (error != GL_NO_ERROR) {
        oss << " 0x" << std::hex << std::uppercase << static_cast<unsigned int>(error);
        error = glGetError();
    }

    uiState_.compileStatus = "OpenGL error";
    uiState_.logText = oss.str();
    uiState_.currentTab = 2;
    return false;
}

} // namespace app
