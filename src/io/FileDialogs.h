#pragma once

#include <string>

namespace io {

bool OpenTextFileDialog(std::string& outPath);
bool SaveTextFileDialog(std::string& inOutPath);

bool ReadUtf8TextFile(const std::string& path, std::string& outText, std::string& outError);
bool WriteUtf8TextFile(const std::string& path, const std::string& text, std::string& outError);

bool IsSupportedTextExtension(const std::string& path);

} // namespace io
