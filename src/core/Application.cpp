#include "core/Application.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

#include <glad/gl.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <windows.h>

#include "io/FileDialogs.h"

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
    LoadPersistentState();

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

    previousFrameTime_ = std::chrono::steady_clock::now();
    return true;
}

void Application::Shutdown()
{
    SavePersistentState();

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

void Application::LoadPersistentState()
{
    if (!core::LoadPersistentState(PersistentConfigPath(), persistentState_)) {
        return;
    }

    framebufferWidth_ = (persistentState_.windowWidth > 320) ? persistentState_.windowWidth : 1280;
    framebufferHeight_ = (persistentState_.windowHeight > 240) ? persistentState_.windowHeight : 720;

    uiState_.currentTab = std::clamp(persistentState_.currentTab, 0, 2);
    uiState_.requestTabSwitch = true;
    uiState_.requestedTab = uiState_.currentTab;

    uiState_.param1 = std::clamp(persistentState_.param1, 0.0f, 100.0f);
    uiState_.param2 = std::clamp(persistentState_.param2, 0.0f, 100.0f);
    uiState_.param3 = std::clamp(persistentState_.param3, 0.0f, 100.0f);
    uiState_.isPlaying = persistentState_.playback;

    codeEditor_.SetZoomPercent(persistentState_.editorZoom);
}

void Application::SavePersistentState() const
{
    core::PersistentAppState state = persistentState_;
    if (window_ != nullptr) {
        int windowWidth = state.windowWidth;
        int windowHeight = state.windowHeight;
        glfwGetWindowSize(window_, &windowWidth, &windowHeight);
        state.windowWidth = windowWidth;
        state.windowHeight = windowHeight;
    }

    state.currentTab = std::clamp(uiState_.currentTab, 0, 2);
    state.lastOpenedFile = codeEditor_.GetFilePath();
    state.editorZoom = codeEditor_.GetZoomPercent();
    state.param1 = uiState_.param1;
    state.param2 = uiState_.param2;
    state.param3 = uiState_.param3;
    state.playback = uiState_.isPlaying;

    std::string saveError;
    core::SavePersistentState(PersistentConfigPath(), state, saveError);
}

std::string Application::PersistentConfigPath()
{
    return "config/app_state.cfg";
}

bool Application::CreateScenePipeline()
{
    const shader::PreparedFragmentSource prepared = shaderCompiler_.BuildBuiltInPreparedSource();
    currentShaderSource_ = shaderCompiler_.BuiltInTestShader();

    std::string compileLog;
    if (!quadRenderer_.Initialize(prepared.generatedSource, compileLog)) {
        shaderLog_ = compileLog;
        uiState_.compileStatus = "Renderer init failed";
        uiState_.logText = shaderLog_.empty() ? "Fullscreen renderer setup failed." : shaderLog_;
        uiState_.compileLogs.clear();
        return false;
    }

    uiState_.compileStatus = "Compiled";
    uiState_.logText = "Built-in test shader loaded.";
    uiState_.compileLogs.clear();
    uiState_.compileDurationMs = 0.0;
    uiState_.sceneViewportWidth = framebufferWidth_;
    uiState_.sceneViewportHeight = framebufferHeight_;

    std::string startupPath = persistentState_.lastOpenedFile;
    if (startupPath.empty()) {
        startupPath = "shaders/default.glsl";
    }

    std::string fileSource;
    std::string fileError;
    if (io::IsSupportedTextExtension(startupPath) &&
        io::ReadUtf8TextFile(startupPath, fileSource, fileError) &&
        !fileSource.empty()) {
        codeEditor_.OpenDocument(startupPath, fileSource);
        currentShaderSource_ = fileSource;
    } else {
        codeEditor_.NewDocument(currentShaderSource_);
    }

    uiState_.sourceCharacterCount = codeEditor_.CharacterCount();
    return true;
}

void Application::ProcessInput()
{
    glfwPollEvents();

    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }

    auto requestTabSwitch = [&](int tabIndex) {
        uiState_.requestTabSwitch = true;
        uiState_.requestedTab = tabIndex;
    };

    const std::vector<core::ShortcutAction> actions = inputManager_.PollActions(window_);
    for (const core::ShortcutAction action : actions) {
        switch (action) {
        case core::ShortcutAction::TabScene:
            requestTabSwitch(0);
            break;
        case core::ShortcutAction::TabCode:
            requestTabSwitch(1);
            break;
        case core::ShortcutAction::TabLogs:
            requestTabSwitch(2);
            break;
        case core::ShortcutAction::NewFile:
            uiState_.requestNewFile = true;
            requestTabSwitch(1);
            break;
        case core::ShortcutAction::OpenFile:
            uiState_.requestOpenFile = true;
            requestTabSwitch(1);
            break;
        case core::ShortcutAction::SaveFile:
            uiState_.requestSaveFile = true;
            requestTabSwitch(1);
            break;
        case core::ShortcutAction::SaveAsFile:
            uiState_.requestSaveAsFile = true;
            requestTabSwitch(1);
            break;
        case core::ShortcutAction::Compile:
            uiState_.requestRecompile = true;
            break;
        case core::ShortcutAction::TogglePlayback:
            uiState_.requestTogglePlayback = true;
            requestTabSwitch(0);
            break;
        case core::ShortcutAction::ResetTimer:
            uiState_.requestResetTimer = true;
            requestTabSwitch(0);
            break;
        }
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
    auto saveCurrentDocument = [&](bool forceSaveAs) -> bool {
        std::string path = codeEditor_.GetFilePath();
        if (forceSaveAs || path.empty()) {
            if (!io::SaveTextFileDialog(path)) {
                return false;
            }
        }

        if (!io::IsSupportedTextExtension(path)) {
            uiState_.compileStatus = "Save failed";
            uiState_.logText = "Unsupported file extension. Use .glsl or .txt.";
            uiState_.currentTab = 2;
            uiState_.requestTabSwitch = true;
            uiState_.requestedTab = 2;
            return false;
        }

        std::string ioError;
        if (io::WriteUtf8TextFile(path, codeEditor_.GetText(), ioError)) {
            codeEditor_.MarkSaved(path);
            uiState_.compileStatus = "Saved";
            uiState_.logText = path;
            uiState_.compileLogs.clear();
            return true;
        }

        uiState_.compileStatus = "Save failed";
        uiState_.logText = ioError;
        uiState_.currentTab = 2;
        uiState_.requestTabSwitch = true;
        uiState_.requestedTab = 2;
        return false;
    };

    auto promptSaveIfDirty = [&]() -> bool {
        if (!codeEditor_.IsDirty()) {
            return true;
        }

        const int decision = MessageBoxA(
            nullptr,
            "Current file has unsaved changes.\nSave before continuing?",
            "Unsaved Changes",
            MB_YESNOCANCEL | MB_ICONWARNING | MB_DEFBUTTON1);

        if (decision == IDCANCEL || decision == 0) {
            return false;
        }

        if (decision == IDYES) {
            return saveCurrentDocument(false);
        }

        return true;
    };

    if (uiState_.requestTogglePlayback) {
        uiState_.isPlaying = !uiState_.isPlaying;
        uiState_.requestTogglePlayback = false;
    }

    if (uiState_.requestResetTimer) {
        iTime_ = 0.0f;
        iFrame_ = 0;
        uiState_.requestResetTimer = false;
    }

    if (uiState_.requestNewFile) {
        uiState_.requestNewFile = false;
        if (promptSaveIfDirty()) {
            codeEditor_.NewDocument(shaderCompiler_.BuiltInTestShader());
            uiState_.compileStatus = "New file";
            uiState_.logText = "Default shader template inserted.";
            uiState_.compileLogs.clear();
        }
    }

    if (uiState_.requestOpenFile) {
        uiState_.requestOpenFile = false;

        if (promptSaveIfDirty()) {
            std::string path;
            if (io::OpenTextFileDialog(path)) {
                if (!io::IsSupportedTextExtension(path)) {
                    uiState_.compileStatus = "Open failed";
                    uiState_.logText = "Unsupported file extension. Use .glsl or .txt.";
                    uiState_.currentTab = 2;
                    uiState_.requestTabSwitch = true;
                    uiState_.requestedTab = 2;
                } else {
                    std::string text;
                    std::string ioError;
                    if (io::ReadUtf8TextFile(path, text, ioError)) {
                        codeEditor_.OpenDocument(path, text);
                        uiState_.compileStatus = "Opened";
                        uiState_.logText = path;
                        uiState_.compileLogs.clear();
                    } else {
                        uiState_.compileStatus = "Open failed";
                        uiState_.logText = ioError;
                        uiState_.currentTab = 2;
                        uiState_.requestTabSwitch = true;
                        uiState_.requestedTab = 2;
                    }
                }
            }
        }
    }

    if (uiState_.requestSaveFile) {
        uiState_.requestSaveFile = false;
        if (!codeEditor_.HasFilePath()) {
            uiState_.requestSaveAsFile = true;
        } else {
            saveCurrentDocument(false);
        }
    }

    if (uiState_.requestSaveAsFile) {
        uiState_.requestSaveAsFile = false;
        saveCurrentDocument(true);
    }

    if (uiState_.requestRecompile) {
        uiState_.requestRecompile = false;
        uiState_.compileLogs.clear();
        uiState_.logText.clear();

        std::string userFragment = codeEditor_.GetText();
        if (userFragment.empty()) {
            userFragment = currentShaderSource_.empty() ? shaderCompiler_.BuiltInTestShader() : currentShaderSource_;
        }

        const shader::CompileReport report = shaderCompiler_.CompileAndHotSwap(userFragment);
        uiState_.compileStatus = report.statusText;
        uiState_.compileDurationMs = report.durationMs;
        uiState_.sourceCharacterCount = report.sourceCharacterCount;
        uiState_.compileLogs = report.entries;
        uiState_.logText = report.mergedLogText;

        if (!report.success) {
            uiState_.currentTab = 2;
            uiState_.requestTabSwitch = true;
            uiState_.requestedTab = 2;
        } else {
            currentShaderSource_ = userFragment;
        }
    }

    uiState_.sourceCharacterCount = codeEditor_.CharacterCount();
}

void Application::UpdateUniforms()
{
    renderState_.iTime = iTime_;
    renderState_.iDeltaTime = iDeltaTime_;
    renderState_.iFrame = static_cast<int>(iFrame_);
    renderState_.iResolutionX = static_cast<float>(uiState_.sceneViewportWidth > 0 ? uiState_.sceneViewportWidth : framebufferWidth_);
    renderState_.iResolutionY = static_cast<float>(uiState_.sceneViewportHeight > 0 ? uiState_.sceneViewportHeight : framebufferHeight_);
    renderState_.param1 = uiState_.param1;
    renderState_.param2 = uiState_.param2;
    renderState_.param3 = uiState_.param3;
}

void Application::RenderScene()
{
    glViewport(0, 0, framebufferWidth_, framebufferHeight_);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    quadRenderer_.Render(renderState_);
    ConsumeOpenGLErrors("RenderScene");
}

void Application::RenderGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const float fps = (iDeltaTime_ > 0.0f) ? (1.0f / iDeltaTime_) : 0.0f;
    layout_.Render(uiState_, codeEditor_, iTime_, fps, iFrame_);

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
    uiState_.sceneViewportWidth = width;
    uiState_.sceneViewportHeight = height;
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
    uiState_.compileLogs.clear();
    shader::CompileLogEntry entry;
    entry.severity = shader::LogSeverity::Error;
    entry.line = -1;
    entry.message = uiState_.logText;
    uiState_.compileLogs.push_back(std::move(entry));
    uiState_.currentTab = 2;
    uiState_.requestTabSwitch = true;
    uiState_.requestedTab = 2;
    return false;
}

} // namespace app
