#pragma once

#include <string>

namespace core {

struct PersistentAppState {
    int windowWidth = 1280;
    int windowHeight = 720;
    int currentTab = 0;
    std::string lastOpenedFile;
    float editorZoom = 100.0f;
    float param1 = 50.0f;
    float param2 = 50.0f;
    float param3 = 50.0f;
    bool playback = true;
};

bool LoadPersistentState(const std::string& path, PersistentAppState& outState);
bool SavePersistentState(const std::string& path, const PersistentAppState& state, std::string& outError);

} // namespace core
