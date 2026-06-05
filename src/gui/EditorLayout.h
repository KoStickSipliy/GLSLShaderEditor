#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

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
};

class EditorLayout {
public:
    void Render(EditorLayoutState& state, float timeSeconds, float fps, int viewportWidth, int viewportHeight, std::uint64_t frameIndex);
};

} // namespace gui
