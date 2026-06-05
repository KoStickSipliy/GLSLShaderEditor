#include "gui/EditorLayout.h"

#include <imgui.h>

namespace gui {

void EditorLayout::Render(EditorLayoutState& state, float timeSeconds, float fps, std::uint64_t frameIndex)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowBgAlpha(0.0f);

    const ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground;

    ImGui::Begin("MainLayout", nullptr, windowFlags);

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Scene")) {
            state.currentTab = 0;

            const float controlPanelHeight = 200.0f;
            ImGui::BeginChild("SceneViewportOverlay", ImVec2(0.0f, -controlPanelHeight), true, ImGuiWindowFlags_NoBackground);
            {
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                const ImVec2 minPos = ImGui::GetWindowPos();
                const ImVec2 maxPos = ImVec2(minPos.x + ImGui::GetWindowSize().x, minPos.y + ImGui::GetWindowSize().y);
                drawList->AddRectFilled(minPos, maxPos, IM_COL32(0, 0, 0, 22));
                drawList->AddRect(minPos, maxPos, IM_COL32(255, 255, 255, 90), 0.0f, 0, 1.5f);
                drawList->AddText(ImVec2(minPos.x + 12.0f, minPos.y + 10.0f), IM_COL32(255, 255, 255, 230), "Scene Viewport");

                state.sceneViewportWidth = static_cast<int>((maxPos.x - minPos.x) > 1.0f ? (maxPos.x - minPos.x) : 1.0f);
                state.sceneViewportHeight = static_cast<int>((maxPos.y - minPos.y) > 1.0f ? (maxPos.y - minPos.y) : 1.0f);
            }
            ImGui::EndChild();

            ImGui::BeginChild("SceneControls", ImVec2(0.0f, 0.0f), true);
            if (ImGui::Button("Reset Timer")) {
                state.requestResetTimer = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(state.isPlaying ? "Pause" : "Play")) {
                state.requestTogglePlayback = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Compile")) {
                state.requestRecompile = true;
            }

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
            ImGui::EndChild();

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Code")) {
            state.currentTab = 1;
            ImGui::BeginChild("CodePanel", ImVec2(0.0f, 0.0f), true);
            ImGui::Text("Code editor will be enabled in the next phase.");
            ImGui::Text("Compile button is already routed through explicit action.");
            ImGui::Separator();
            ImGui::Text("Last compile: %.3f ms", state.compileDurationMs);
            ImGui::Text("Character count: %llu", static_cast<unsigned long long>(state.sourceCharacterCount));
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Logs")) {
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
    }

    ImGui::End();
}

} // namespace gui
