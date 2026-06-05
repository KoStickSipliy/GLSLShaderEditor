#include "core/PersistentConfig.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <exception>

namespace {

bool ParseInt(const std::unordered_map<std::string, std::string>& values, const char* key, int& outValue)
{
    const auto it = values.find(key);
    if (it == values.end()) {
        return false;
    }
    try {
        outValue = std::stoi(it->second);
        return true;
    } catch (...) {
        return false;
    }
}

bool ParseFloat(const std::unordered_map<std::string, std::string>& values, const char* key, float& outValue)
{
    const auto it = values.find(key);
    if (it == values.end()) {
        return false;
    }
    try {
        outValue = std::stof(it->second);
        return true;
    } catch (...) {
        return false;
    }
}

bool ParseBool(const std::unordered_map<std::string, std::string>& values, const char* key, bool& outValue)
{
    const auto it = values.find(key);
    if (it == values.end()) {
        return false;
    }
    outValue = it->second == "1" || it->second == "true" || it->second == "True";
    return true;
}

} // namespace

namespace core {

bool LoadPersistentState(const std::string& path, PersistentAppState& outState)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::unordered_map<std::string, std::string> values;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        const std::size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }
        values[line.substr(0, eqPos)] = line.substr(eqPos + 1);
    }

    ParseInt(values, "window_width", outState.windowWidth);
    ParseInt(values, "window_height", outState.windowHeight);
    ParseInt(values, "current_tab", outState.currentTab);
    ParseFloat(values, "editor_zoom", outState.editorZoom);
    ParseFloat(values, "param1", outState.param1);
    ParseFloat(values, "param2", outState.param2);
    ParseFloat(values, "param3", outState.param3);
    ParseBool(values, "playback", outState.playback);

    const auto lastFileIt = values.find("last_opened_file");
    if (lastFileIt != values.end()) {
        outState.lastOpenedFile = lastFileIt->second;
    }

    return true;
}

bool SavePersistentState(const std::string& path, const PersistentAppState& state, std::string& outError)
{
    try {
        const std::filesystem::path fsPath(path);
        const std::filesystem::path parent = fsPath.parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }
    } catch (const std::exception& ex) {
        outError = ex.what();
        return false;
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        outError = "Failed to open config file for writing.";
        return false;
    }

    file << "window_width=" << state.windowWidth << "\n";
    file << "window_height=" << state.windowHeight << "\n";
    file << "current_tab=" << state.currentTab << "\n";
    file << "last_opened_file=" << state.lastOpenedFile << "\n";
    file << "editor_zoom=" << state.editorZoom << "\n";
    file << "param1=" << state.param1 << "\n";
    file << "param2=" << state.param2 << "\n";
    file << "param3=" << state.param3 << "\n";
    file << "playback=" << (state.playback ? 1 : 0) << "\n";

    if (!file.good()) {
        outError = "Failed to write config file.";
        return false;
    }

    outError.clear();
    return true;
}

} // namespace core
