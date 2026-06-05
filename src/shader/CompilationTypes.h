#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace shader {

enum class LogSeverity {
    Info,
    Warning,
    Error
};

struct CompileLogEntry {
    LogSeverity severity = LogSeverity::Info;
    int line = -1;
    std::string message;
};

struct LineRemapContext {
    bool usesLineDirective = false;
    int generatedUserStartLine = 1;
    int generatedWrapperStartLine = 1;
};

struct PreparedFragmentSource {
    std::string generatedSource;
    std::string preprocessedUserSource;
    LineRemapContext remapContext;
    std::size_t userCharacterCount = 0;
};

struct CompileReport {
    bool success = false;
    double durationMs = 0.0;
    std::string statusText;

    std::vector<CompileLogEntry> entries;
    std::string mergedLogText;

    std::size_t sourceCharacterCount = 0;
    std::string generatedFragmentSource;
};

} // namespace shader
