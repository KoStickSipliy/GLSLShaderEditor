#include "core/Application.h"

#include <fstream>
#include <sstream>

#include <glad/glad.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace {

const char* kVertexShaderSource = R"(#version 330 core
layout(location = 0) in vec2 aPosition;

void main()
{
    gl_Position = vec4(aPosition, 0.0, 1.0);
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
    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == 0) {
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
    std::string userFragment = ReadUtf8TextFile("shaders/default.glsl");
    if (userFragment.empty()) {
        userFragment = R"(
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    vec3 col = vec3(uv, 0.5 + 0.5 * sin(iTime));
    fragColor = vec4(col, 1.0);
}
)";
    }

    std::string compileLog;

    if (!vertexShader_.CompileFromSource(GL_VERTEX_SHADER, kVertexShaderSource, compileLog)) {
        shaderLog_ = compileLog;
        uiState_.compileStatus = "Vertex compile failed";
        uiState_.logText = shaderLog_;
        return false;
    }

    const std::string fragmentSource = BuildFragmentSource(userFragment);
    if (!fragmentShader_.CompileFromSource(GL_FRAGMENT_SHADER, fragmentSource, compileLog)) {
        shaderLog_ = compileLog;
        uiState_.compileStatus = "Fragment compile failed";
        uiState_.logText = shaderLog_;
        return false;
    }

    if (!shaderProgram_.Link(vertexShader_, fragmentShader_, compileLog)) {
        shaderLog_ = compileLog;
        uiState_.compileStatus = "Link failed";
        uiState_.logText = shaderLog_;
        return false;
    }

    if (!fullscreenQuad_.Initialize()) {
        uiState_.compileStatus = "Geometry init failed";
        uiState_.logText = "Fullscreen quad setup failed.";
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
        uiState_.compileStatus = "Compile on demand (Phase 2)";
        uiState_.logText = "Compilation trigger captured in Phase 1.";
    }
}

void Application::UpdateUniforms()
{
}

void Application::RenderScene()
{
    glViewport(0, 0, framebufferWidth_, framebufferHeight_);
    glClearColor(0.08f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shaderProgram_.Use();
    shaderProgram_.SetFloat("iTime", iTime_);
    shaderProgram_.SetFloat("iDeltaTime", iDeltaTime_);
    shaderProgram_.SetInt("iFrame", static_cast<int>(iFrame_));
    shaderProgram_.SetVec2("iResolution", static_cast<float>(framebufferWidth_), static_cast<float>(framebufferHeight_));
    shaderProgram_.SetVec4("iMouse", 0.0f, 0.0f, 0.0f, 0.0f);
    shaderProgram_.SetFloat("PARAM1", uiState_.param1);
    shaderProgram_.SetFloat("PARAM2", uiState_.param2);
    shaderProgram_.SetFloat("PARAM3", uiState_.param3);

    fullscreenQuad_.Draw();
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

} // namespace app
