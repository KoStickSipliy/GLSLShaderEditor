#include "io/FileDialogs.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>

#include <windows.h>
#include <commdlg.h>

namespace {

std::string ToLower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

std::string GetExtensionLower(const std::string& path)
{
    const std::size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) {
        return {};
    }
    return ToLower(path.substr(dotPos));
}

} // namespace

namespace io {

bool OpenTextFileDialog(std::string& outPath)
{
    char pathBuffer[MAX_PATH] = {};

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = pathBuffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter =
        "Shader Files (*.glsl;*.txt)\0*.glsl;*.txt\0"
        "GLSL Files (*.glsl)\0*.glsl\0"
        "Text Files (*.txt)\0*.txt\0"
        "All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == FALSE) {
        return false;
    }

    outPath = pathBuffer;
    return true;
}

bool SaveTextFileDialog(std::string& inOutPath)
{
    char pathBuffer[MAX_PATH] = {};
    if (!inOutPath.empty()) {
        const std::size_t copySize = (std::min)(inOutPath.size(), static_cast<std::size_t>(MAX_PATH - 1));
        memcpy(pathBuffer, inOutPath.c_str(), copySize);
        pathBuffer[copySize] = '\0';
    }

    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFile = pathBuffer;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter =
        "GLSL Files (*.glsl)\0*.glsl\0"
        "Text Files (*.txt)\0*.txt\0"
        "All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "glsl";

    if (GetSaveFileNameA(&ofn) == FALSE) {
        return false;
    }

    inOutPath = pathBuffer;
    if (GetExtensionLower(inOutPath).empty()) {
        inOutPath += ".glsl";
    }
    return true;
}

bool ReadUtf8TextFile(const std::string& path, std::string& outText, std::string& outError)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        outError = "Failed to open file for reading: " + path;
        return false;
    }

    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();
    file.seekg(0, std::ios::beg);

    outText.clear();
    if (size > 0) {
        outText.resize(static_cast<std::size_t>(size));
        file.read(outText.data(), size);
    }

    if (!file.good() && !file.eof()) {
        outError = "Failed to read file: " + path;
        return false;
    }

    outError.clear();
    return true;
}

bool WriteUtf8TextFile(const std::string& path, const std::string& text, std::string& outError)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        outError = "Failed to open file for writing: " + path;
        return false;
    }

    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!file.good()) {
        outError = "Failed to write file: " + path;
        return false;
    }

    outError.clear();
    return true;
}

bool IsSupportedTextExtension(const std::string& path)
{
    const std::string extension = GetExtensionLower(path);
    return extension == ".glsl" || extension == ".txt";
}

} // namespace io
