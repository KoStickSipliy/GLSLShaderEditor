#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

#include "editor/CodeEditor.h"
#include "shader/CompilationTypes.h"

namespace gui {

struct EditorLayoutState {
    int currentTab = 0;
    bool isPlaying = true;

    float param1 = 50.0f;
    float param2 = 50.0f;
    float param3 = 50.0f;

    std::string compileStatus = "Idle";
    std::string logText;
    std::vector<shader::CompileLogEntry> compileLogs;
    double compileDurationMs = 0.0;
    std::size_t sourceCharacterCount = 0;

    bool requestResetTimer = false;
    bool requestTogglePlayback = false;
    bool requestRecompile = false;
    bool requestNewFile = false;
    bool requestOpenFile = false;
    bool requestSaveFile = false;
    bool requestSaveAsFile = false;

    int sceneViewportWidth = 1;
    int sceneViewportHeight = 1;
};

class EditorLayout {
public:
    void Render(EditorLayoutState& state, editor::CodeEditor& codeEditor, float timeSeconds, float fps, std::uint64_t frameIndex);
};

} // namespace gui
