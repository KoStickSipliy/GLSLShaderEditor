#include "gui/EditorLayout.h"

#include <imgui.h>

namespace gui {

void EditorLayout::Render(EditorLayoutState& state, float timeSeconds, float fps, int viewportWidth, int viewportHeight, std::uint64_t frameIndex)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    const ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("MainLayout", nullptr, windowFlags);

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Scene")) {
            state.currentTab = 0;

            if (ImGui::Button("Reset Timer")) {
                state.requestResetTimer = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(state.isPlaying ? "Pause" : "Play")) {
                state.requestTogglePlayback = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Recompile")) {
                state.requestRecompile = true;
            }

            ImGui::Separator();
            ImGui::Text("Status: %s", state.compileStatus.c_str());
            ImGui::Text("Time: %.3f", timeSeconds);
            ImGui::Text("Resolution: %d x %d", viewportWidth, viewportHeight);
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

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Code")) {
            state.currentTab = 1;
            ImGui::Text("Code editor will be enabled in the next phase.");
            ImGui::Text("Compile button is already routed through explicit action.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Logs")) {
            state.currentTab = 2;
            ImGui::TextUnformatted(state.logText.c_str());
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace gui
