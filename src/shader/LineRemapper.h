#pragma once

#include <string>
#include <vector>

#include "shader/CompilationTypes.h"

namespace shader {

class LineRemapper {
public:
    std::vector<CompileLogEntry> Remap(const std::string& compilerLog, const LineRemapContext& context) const;
    std::string MergeToText(const std::vector<CompileLogEntry>& entries) const;

private:
    static std::vector<std::string> SplitLines(const std::string& text);
    static LogSeverity DetectSeverity(const std::string& line);
};

} // namespace shader
