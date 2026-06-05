#include "editor/CodeEditor.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

#include <imgui.h>

#include <TextEditor.h>

namespace {

constexpr std::array<float, 7> kZoomLevels{50.0f, 75.0f, 100.0f, 125.0f, 150.0f, 175.0f, 200.0f};

std::string GetFileNameFromPath(const std::string& path)
{
    const std::size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

} // namespace

namespace editor {

CodeEditor::CodeEditor()
    : textEditor_(new TextEditor())
{
    textEditor_->SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
    textEditor_->SetPalette(TextEditor::GetDarkPalette());
    textEditor_->SetShowWhitespaces(false);
    textEditor_->SetTabSize(4);
    textEditor_->SetText({});
}

CodeEditor::~CodeEditor()
{
    delete textEditor_;
    textEditor_ = nullptr;
}

void CodeEditor::NewDocument(const std::string& templateText)
{
    filePath_.clear();
    textEditor_->SetText(templateText);
    RefreshCachedText();
    cleanSnapshot_ = textCache_;
    dirty_ = false;
}

void CodeEditor::OpenDocument(const std::string& path, const std::string& text)
{
    filePath_ = path;
    textEditor_->SetText(text);
    RefreshCachedText();
    cleanSnapshot_ = textCache_;
    dirty_ = false;
}

void CodeEditor::MarkSaved(const std::string& path)
{
    filePath_ = path;
    RefreshCachedText();
    cleanSnapshot_ = textCache_;
    dirty_ = false;
}

void CodeEditor::Render(
    const std::string& compileStatus,
    double compileDurationMs,
    bool& requestCompile,
    bool& requestNewFile,
    bool& requestOpenFile,
    bool& requestSaveFile,
    bool& requestSaveAsFile,
    std::size_t& outCharacterCount)
{
    const float panelHeight = (ImGui::GetFrameHeightWithSpacing() * 2.0f) + 28.0f;

    ImGui::BeginChild("CodeEditorRoot", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                requestNewFile = true;
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                requestOpenFile = true;
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                requestSaveFile = true;
            }
            if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {
                requestSaveAsFile = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Zoom")) {
            for (const float level : kZoomLevels) {
                char label[16] = {};
                snprintf(label, sizeof(label), "%.0f%%", level);
                if (ImGui::MenuItem(label, nullptr, std::abs(zoomPercent_ - level) < 0.1f)) {
                    SetZoomPercent(level);
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::Text("File: %s", BuildDisplayName().c_str());
    ImGui::SameLine();
    ImGui::Text("Zoom: %.0f%%", zoomPercent_);

    ImGui::BeginChild("CodeEditorArea", ImVec2(0.0f, -panelHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
        ImGuiIO& io = ImGui::GetIO();
        const float oldGlobalScale = io.FontGlobalScale;
        io.FontGlobalScale = zoomPercent_ / 100.0f;
        textEditor_->Render("ShaderTextEditorWidget", ImVec2(-1.0f, -1.0f), false);
        io.FontGlobalScale = oldGlobalScale;

        if (textEditor_->IsTextChanged()) {
            RefreshCachedText();
            dirty_ = (textCache_ != cleanSnapshot_);
        }
    }
    ImGui::EndChild();

    ImGui::BeginChild("CodeCompilePanel", ImVec2(0.0f, 0.0f), true);
    if (ImGui::Button("Compile")) {
        requestCompile = true;
    }
    ImGui::SameLine();
    ImGui::Text("Status: %s", compileStatus.c_str());
    ImGui::Text("Compile: %.3f ms", compileDurationMs);
    ImGui::SameLine();
    ImGui::Text("Chars: %llu", static_cast<unsigned long long>(textCache_.size()));
    ImGui::EndChild();

    outCharacterCount = textCache_.size();
    ImGui::EndChild();
}

std::string CodeEditor::GetText() const
{
    return textEditor_->GetText();
}

void CodeEditor::SetZoomPercent(float zoomPercent)
{
    if (zoomPercent < 50.0f) {
        zoomPercent_ = 50.0f;
    } else if (zoomPercent > 200.0f) {
        zoomPercent_ = 200.0f;
    } else {
        zoomPercent_ = zoomPercent;
    }
}

void CodeEditor::RefreshCachedText()
{
    textCache_ = textEditor_->GetText();
}

std::string CodeEditor::BuildDisplayName() const
{
    std::string name = filePath_.empty() ? "Untitled.glsl" : GetFileNameFromPath(filePath_);
    if (dirty_) {
        name += " *";
    }
    return name;
}

} // namespace editor
