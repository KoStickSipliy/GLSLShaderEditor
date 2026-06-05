#include "core/InputManager.h"

#include <array>

namespace {

struct ShortcutBinding {
    core::ShortcutAction action;
    int key;
    bool ctrl;
    bool shift;
};

constexpr std::array<ShortcutBinding, 11> kBindings{{
    {core::ShortcutAction::TabScene, GLFW_KEY_1, true, false},
    {core::ShortcutAction::TabCode, GLFW_KEY_2, true, false},
    {core::ShortcutAction::TabLogs, GLFW_KEY_3, true, false},
    {core::ShortcutAction::NewFile, GLFW_KEY_N, true, false},
    {core::ShortcutAction::OpenFile, GLFW_KEY_O, true, false},
    {core::ShortcutAction::SaveFile, GLFW_KEY_S, true, false},
    {core::ShortcutAction::SaveAsFile, GLFW_KEY_S, true, true},
    {core::ShortcutAction::Compile, GLFW_KEY_F5, true, false},
    {core::ShortcutAction::TogglePlayback, GLFW_KEY_SPACE, true, false},
    {core::ShortcutAction::ResetTimer, GLFW_KEY_T, true, false},
    {core::ShortcutAction::ToggleFullscreen, GLFW_KEY_F, false, false},
}};

bool IsCtrlPressed(GLFWwindow* window)
{
    return glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
}

bool IsShiftPressed(GLFWwindow* window)
{
    return glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
}

} // namespace

namespace core {

std::vector<ShortcutAction> InputManager::PollActions(GLFWwindow* window)
{
    std::vector<ShortcutAction> actions;
    actions.reserve(kBindings.size());

    const bool ctrl = IsCtrlPressed(window);
    const bool shift = IsShiftPressed(window);

    for (std::size_t i = 0; i < kBindings.size(); ++i) {
        const ShortcutBinding& binding = kBindings[i];
        const bool pressed = glfwGetKey(window, binding.key) == GLFW_PRESS;
        const bool chordDown = pressed && (ctrl == binding.ctrl) && (shift == binding.shift);
        if (chordDown && !previousDown_[i]) {
            actions.push_back(binding.action);
        }
        previousDown_[i] = chordDown;
    }

    return actions;
}

} // namespace core
