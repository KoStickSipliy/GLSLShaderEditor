#pragma once

#include <cstddef>
#include <string>

class TextEditor;

namespace editor {

class CodeEditor {
public:
    CodeEditor();
    ~CodeEditor();

    CodeEditor(const CodeEditor&) = delete;
    CodeEditor& operator=(const CodeEditor&) = delete;

    void NewDocument(const std::string& templateText);
    void OpenDocument(const std::string& path, const std::string& text);
    void MarkSaved(const std::string& path);

    void Render(
        const std::string& compileStatus,
        double compileDurationMs,
        bool& requestCompile,
        bool& requestNewFile,
        bool& requestOpenFile,
        bool& requestSaveFile,
        bool& requestSaveAsFile,
        std::size_t& outCharacterCount);

    std::string GetText() const;
    const std::string& GetFilePath() const { return filePath_; }
    bool HasFilePath() const { return !filePath_.empty(); }
    bool IsDirty() const { return dirty_; }
    std::size_t CharacterCount() const { return textCache_.size(); }
    float GetZoomPercent() const { return zoomPercent_; }
    void SetZoomPercent(float zoomPercent);

private:
    void RefreshCachedText();
    std::string BuildDisplayName() const;

private:
    TextEditor* textEditor_ = nullptr;

    std::string filePath_;
    std::string textCache_;
    std::string cleanSnapshot_;

    float zoomPercent_ = 100.0f;
    bool dirty_ = false;
};

} // namespace editor
