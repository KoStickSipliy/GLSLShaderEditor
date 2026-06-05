#include "core/Application.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include <glad/gl.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <windows.h>

#include "graphics/GLDebug.h"
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

    quadRenderer_.Shutdown();
    ValidateResourceSnapshot("Shutdown", graphics::GLResourceSnapshot{}, false);
    activeResourceSnapshotValid_ = false;

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
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

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
    graphics::InitializeGLDebugOutput();
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
        SetLogStatus(
            "Renderer init failed",
            shaderLog_.empty() ? "Fullscreen renderer setup failed." : shaderLog_,
            shader::LogSeverity::Error,
            false);
        return false;
    }

    uiState_.compileStatus = "Compiled";
    uiState_.logText = "Built-in test shader loaded.";
    uiState_.compileLogs.clear();
    uiState_.compileDurationMs = 0.0;
    uiState_.sceneViewportWidth = framebufferWidth_;
    uiState_.sceneViewportHeight = framebufferHeight_;
    activeResourceSnapshot_ = graphics::GLResourceDiagnostics::Snapshot();
    activeResourceSnapshotValid_ = true;

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

    const std::vector<core::ShortcutAction> actions = inputManager_.PollActions(window_);
    for (const core::ShortcutAction action : actions) {
        switch (action) {
        case core::ShortcutAction::TabScene:
            RequestTabSwitch(0);
            break;
        case core::ShortcutAction::TabCode:
            RequestTabSwitch(1);
            break;
        case core::ShortcutAction::TabLogs:
            RequestTabSwitch(2);
            break;
        case core::ShortcutAction::NewFile:
            uiState_.requestNewFile = true;
            RequestTabSwitch(1);
            break;
        case core::ShortcutAction::OpenFile:
            uiState_.requestOpenFile = true;
            RequestTabSwitch(1);
            break;
        case core::ShortcutAction::SaveFile:
            uiState_.requestSaveFile = true;
            RequestTabSwitch(1);
            break;
        case core::ShortcutAction::SaveAsFile:
            uiState_.requestSaveAsFile = true;
            RequestTabSwitch(1);
            break;
        case core::ShortcutAction::Compile:
            uiState_.requestRecompile = true;
            break;
        case core::ShortcutAction::TogglePlayback:
            uiState_.requestTogglePlayback = true;
            RequestTabSwitch(0);
            break;
        case core::ShortcutAction::ResetTimer:
            uiState_.requestResetTimer = true;
            RequestTabSwitch(0);
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
            SetLogStatus(
                "Save failed",
                "Unsupported file extension. Use .glsl or .txt.",
                shader::LogSeverity::Error,
                true);
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

        SetLogStatus("Save failed", ioError, shader::LogSeverity::Error, true);
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
                    SetLogStatus(
                        "Open failed",
                        "Unsupported file extension. Use .glsl or .txt.",
                        shader::LogSeverity::Error,
                        true);
                } else {
                    std::string text;
                    std::string ioError;
                    if (io::ReadUtf8TextFile(path, text, ioError)) {
                        codeEditor_.OpenDocument(path, text);
                        uiState_.compileStatus = "Opened";
                        uiState_.logText = path;
                        uiState_.compileLogs.clear();
                    } else {
                        SetLogStatus("Open failed", ioError, shader::LogSeverity::Error, true);
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
            RequestTabSwitch(2);
        } else {
            currentShaderSource_ = userFragment;
        }

        if (activeResourceSnapshotValid_) {
            ValidateResourceSnapshot("Recompile", activeResourceSnapshot_, true);
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

    double cursorX = 0.0;
    double cursorY = 0.0;
    glfwGetCursorPos(window_, &cursorX, &cursorY);

    const float clampedMouseX = std::clamp(static_cast<float>(cursorX), 0.0f, renderState_.iResolutionX);
    const float clampedMouseY = std::clamp(renderState_.iResolutionY - static_cast<float>(cursorY), 0.0f, renderState_.iResolutionY);

    const bool mousePressed = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (mousePressed && !leftMouseDown_) {
        mouseDownX_ = clampedMouseX;
        mouseDownY_ = clampedMouseY;
    }
    leftMouseDown_ = mousePressed;

    renderState_.iMouseX = clampedMouseX;
    renderState_.iMouseY = clampedMouseY;
    renderState_.iMouseZ = mousePressed ? mouseDownX_ : 0.0f;
    renderState_.iMouseW = mousePressed ? mouseDownY_ : 0.0f;
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

void Application::RequestTabSwitch(int tabIndex)
{
    uiState_.requestTabSwitch = true;
    uiState_.requestedTab = tabIndex;
}

void Application::SetLogStatus(const std::string& status, const std::string& text, shader::LogSeverity severity, bool focusLogs)
{
    uiState_.compileStatus = status;
    uiState_.logText = text;
    uiState_.compileLogs.clear();

    shader::CompileLogEntry entry;
    entry.severity = severity;
    entry.line = -1;
    entry.message = text;
    uiState_.compileLogs.push_back(std::move(entry));

    if (focusLogs) {
        RequestTabSwitch(2);
    }
}

bool Application::ValidateResourceSnapshot(const char* stage, const graphics::GLResourceSnapshot& expected, bool focusLogs)
{
    const graphics::GLResourceSnapshot current = graphics::GLResourceDiagnostics::Snapshot();
    if (graphics::GLResourceDiagnostics::Matches(current, expected)) {
        return true;
    }

    std::ostringstream oss;
    oss << stage << " resource mismatch. expected("
        << graphics::GLResourceDiagnostics::ToString(expected) << "), got("
        << graphics::GLResourceDiagnostics::ToString(current) << ")";

    if (focusLogs) {
        SetLogStatus("Resource warning", oss.str(), shader::LogSeverity::Warning, true);
    } else {
        std::string line = oss.str() + "\n";
        OutputDebugStringA(line.c_str());
    }

    return false;
}

bool Application::ConsumeOpenGLErrors(const char* stage)
{
    std::string apiErrorMessage;
    const bool hasApiError = graphics::DrainOpenGLErrors(stage, apiErrorMessage);
    const std::vector<graphics::GLDebugMessage> debugMessages = graphics::ConsumeGLDebugMessages();

    bool hasDebugIssue = false;
    for (const graphics::GLDebugMessage& message : debugMessages) {
        if (message.severity == graphics::GLDebugSeverity::Warning ||
            message.severity == graphics::GLDebugSeverity::Error) {
            hasDebugIssue = true;
            break;
        }
    }

    if (!hasApiError && !hasDebugIssue) {
        return true;
    }

    std::ostringstream oss;
    bool hasAnyMessage = false;
    if (hasApiError) {
        oss << apiErrorMessage;
        hasAnyMessage = true;
    }

    for (const graphics::GLDebugMessage& message : debugMessages) {
        if (message.severity == graphics::GLDebugSeverity::Info) {
            continue;
        }
        if (hasAnyMessage) {
            oss << '\n';
        }
        const char* severityText = message.severity == graphics::GLDebugSeverity::Error ? "ERROR" : "WARNING";
        oss << stage << " GL debug " << severityText << " [" << message.id << "]: " << message.text;
        hasAnyMessage = true;
    }

    const shader::LogSeverity severity = hasApiError ? shader::LogSeverity::Error : shader::LogSeverity::Warning;
    SetLogStatus(
        hasApiError ? "OpenGL error" : "OpenGL warning",
        oss.str(),
        severity,
        hasApiError);
    return !hasApiError;
}

} // namespace app
