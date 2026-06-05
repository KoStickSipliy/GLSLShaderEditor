#pragma once

#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace core {

enum class ShortcutAction {
    TabScene,
    TabCode,
    TabLogs,
    NewFile,
    OpenFile,
    SaveFile,
    SaveAsFile,
    Compile,
    TogglePlayback,
    ResetTimer,
    ToggleFullscreen
};

class InputManager {
public:
    std::vector<ShortcutAction> PollActions(GLFWwindow* window);

private:
    bool previousDown_[11] = {};
};

} // namespace core
