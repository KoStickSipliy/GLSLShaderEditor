#include "gui/EditorLayout.h"

#include <imgui.h>

namespace gui {

void EditorLayout::Render(EditorLayoutState& state, editor::CodeEditor& codeEditor, float timeSeconds, float fps, std::uint64_t frameIndex)
{
    ImVec4 opaqueChildBg = ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
    opaqueChildBg.w = 1.0f;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, opaqueChildBg);

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    const ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground;

    ImGui::Begin("MainLayout", nullptr, windowFlags);

    auto showShortcutTooltip = [](const char* shortcutText) {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
            ImGui::SetTooltip("%s", shortcutText);
        }
    };

    if (ImGui::BeginTabBar("MainTabs")) {
        ImGuiTabItemFlags sceneTabFlags = 0;
        ImGuiTabItemFlags codeTabFlags = 0;
        ImGuiTabItemFlags logsTabFlags = 0;
        if (state.requestTabSwitch) {
            if (state.requestedTab == 0) {
                sceneTabFlags |= ImGuiTabItemFlags_SetSelected;
            } else if (state.requestedTab == 1) {
                codeTabFlags |= ImGuiTabItemFlags_SetSelected;
            } else if (state.requestedTab == 2) {
                logsTabFlags |= ImGuiTabItemFlags_SetSelected;
            }
        }

        if (ImGui::BeginTabItem("Scene", nullptr, sceneTabFlags)) {
            state.currentTab = 0;

            const float rowHeight = ImGui::GetFrameHeightWithSpacing();
            const ImGuiStyle& style = ImGui::GetStyle();
            const float toolbarHeight = rowHeight * 12.0f + style.ItemSpacing.y * 2.0f + style.WindowPadding.y * 2.0f;
            ImGui::BeginChild(
                "SceneViewportOverlay",
                ImVec2(0.0f, -toolbarHeight),
                true,
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            {
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                const ImVec2 minPos = ImGui::GetWindowPos();
                const ImVec2 maxPos = ImVec2(minPos.x + ImGui::GetWindowSize().x, minPos.y + ImGui::GetWindowSize().y);
                drawList->AddRect(minPos, maxPos, IM_COL32(255, 255, 255, 140), 0.0f, 0, 1.5f);
                drawList->AddText(ImVec2(minPos.x + 12.0f, minPos.y + 10.0f), IM_COL32(255, 255, 255, 230), "Scene Viewport");

                state.sceneViewportPosX = static_cast<int>(minPos.x);
                state.sceneViewportPosY = static_cast<int>(minPos.y);
                state.sceneViewportWidth = static_cast<int>((maxPos.x - minPos.x) > 1.0f ? (maxPos.x - minPos.x) : 1.0f);
                state.sceneViewportHeight = static_cast<int>((maxPos.y - minPos.y) > 1.0f ? (maxPos.y - minPos.y) : 1.0f);

                const float buttonWidth = 34.0f;
                const float buttonHeight = ImGui::GetFrameHeight();
                ImGui::SetCursorScreenPos(ImVec2(maxPos.x - buttonWidth - 10.0f, maxPos.y - buttonHeight - 10.0f));
                if (ImGui::Button("FS", ImVec2(buttonWidth, buttonHeight))) {
                    state.requestToggleFullscreen = true;
                }
                showShortcutTooltip("F");
            }
            ImGui::EndChild();

            ImGui::BeginChild(
                "SceneControls",
                ImVec2(0.0f, toolbarHeight),
                true,
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            if (ImGui::Button("Reset Timer")) {
                state.requestResetTimer = true;
            }
            showShortcutTooltip("Ctrl+T");
            ImGui::SameLine();
            if (ImGui::Button(state.isPlaying ? "Pause" : "Play")) {
                state.requestTogglePlayback = true;
            }
            showShortcutTooltip("Ctrl+Space");
            ImGui::SameLine();
            if (ImGui::Button("Compile")) {
                state.requestRecompile = true;
            }
            showShortcutTooltip("Ctrl+F5");

            ImGui::Separator();
            ImVec4 statusColor = ImVec4(0.70f, 0.90f, 0.70f, 1.0f);
            if (state.compileStatus.find("failed") != std::string::npos || state.compileStatus.find("error") != std::string::npos ||
                state.compileStatus.find("Failed") != std::string::npos || state.compileStatus.find("Error") != std::string::npos) {
                statusColor = ImVec4(1.0f, 0.45f, 0.45f, 1.0f);
            }
            ImGui::TextColored(statusColor, "Status: %s", state.compileStatus.c_str());
            ImGui::Text("Compile: %.3f ms", state.compileDurationMs);
            ImGui::Text("Time: %.3f", timeSeconds);
            ImGui::Text("Resolution: %d x %d", state.sceneViewportWidth, state.sceneViewportHeight);
            ImGui::Text("FPS: %.1f", fps);
            ImGui::Text("Frame: %llu", static_cast<unsigned long long>(frameIndex));

            ImGui::Separator();
            ImGui::SliderFloat("PARAM1", &state.param1, 0.0f, 100.0f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90.0f);
            ImGui::InputFloat("##PARAM1_INPUT", &state.param1, 0.1f, 1.0f, "%.2f");
            state.param1 = (state.param1 < 0.0f) ? 0.0f : ((state.param1 > 100.0f) ? 100.0f : state.param1);

            ImGui::SliderFloat("PARAM2", &state.param2, 0.0f, 100.0f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90.0f);
            ImGui::InputFloat("##PARAM2_INPUT", &state.param2, 0.1f, 1.0f, "%.2f");
            state.param2 = (state.param2 < 0.0f) ? 0.0f : ((state.param2 > 100.0f) ? 100.0f : state.param2);

            ImGui::SliderFloat("PARAM3", &state.param3, 0.0f, 100.0f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90.0f);
            ImGui::InputFloat("##PARAM3_INPUT", &state.param3, 0.1f, 1.0f, "%.2f");
            state.param3 = (state.param3 < 0.0f) ? 0.0f : ((state.param3 > 100.0f) ? 100.0f : state.param3);

            if (state.isFullscreen) {
                ImGui::Separator();
                ImGui::TextDisabled("Esc - exit fullscreen");
            }
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Code", nullptr, codeTabFlags)) {
            state.currentTab = 1;
            codeEditor.Render(
                state.compileStatus,
                state.compileDurationMs,
                state.requestRecompile,
                state.requestNewFile,
                state.requestOpenFile,
                state.requestSaveFile,
                state.requestSaveAsFile,
                state.sourceCharacterCount);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Logs", nullptr, logsTabFlags)) {
            state.currentTab = 2;
            ImGui::BeginChild("LogsPanel", ImVec2(0.0f, 0.0f), true);
            if (state.compileLogs.empty()) {
                ImGui::TextUnformatted(state.logText.c_str());
            } else {
                for (const shader::CompileLogEntry& entry : state.compileLogs) {
                    const char* sev = "INFO";
                    if (entry.severity == shader::LogSeverity::Warning) {
                        sev = "WARNING";
                    } else if (entry.severity == shader::LogSeverity::Error) {
                        sev = "ERROR";
                    }

                    if (entry.line > 0) {
                        ImGui::Text("[%s] line %d: %s", sev, entry.line, entry.message.c_str());
                    } else {
                        ImGui::Text("[%s] %s", sev, entry.message.c_str());
                    }
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
        state.requestTabSwitch = false;
    }

    ImGui::End();
    ImGui::PopStyleColor(1);
}

} // namespace gui
