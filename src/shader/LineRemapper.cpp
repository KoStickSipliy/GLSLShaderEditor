#include "shader/LineRemapper.h"

#include <regex>
#include <sstream>
#include <utility>

namespace {

int RemapLineNumber(int lineNumber, const shader::LineRemapContext& context)
{
    if (lineNumber <= 0) {
        return -1;
    }

    if (context.usesLineDirective) {
        return lineNumber;
    }

    if (lineNumber >= context.generatedUserStartLine && lineNumber < context.generatedWrapperStartLine) {
        return lineNumber - context.generatedUserStartLine + 1;
    }

    return -1;
}

} // namespace

namespace shader {

std::vector<CompileLogEntry> LineRemapper::Remap(const std::string& compilerLog, const LineRemapContext& context) const
{
    static const std::regex kPatternA(R"((\d+)\((\d+)\)\s*:\s*(warning|error)[^:]*:\s*(.*))", std::regex::icase);
    static const std::regex kPatternB(R"((warning|error)\s*:\s*(\d+)\s*:\s*(\d+)\s*:\s*(.*))", std::regex::icase);
    static const std::regex kPatternC(R"(0:(\d+):\s*(warning|error)\s*:\s*(.*))", std::regex::icase);

    std::vector<CompileLogEntry> entries;
    const std::vector<std::string> lines = SplitLines(compilerLog);
    for (const std::string& line : lines) {
        if (line.empty()) {
            continue;
        }

        std::smatch match;
        CompileLogEntry entry;
        entry.severity = DetectSeverity(line);
        entry.message = line;
        entry.line = -1;

        if (std::regex_search(line, match, kPatternA) && match.size() >= 5) {
            const int generatedLine = std::stoi(match[2].str());
            const std::string sev = match[3].str();
            entry.severity = (sev == "warning" || sev == "WARNING") ? LogSeverity::Warning : LogSeverity::Error;
            entry.line = RemapLineNumber(generatedLine, context);
            entry.message = match[4].str();
        } else if (std::regex_search(line, match, kPatternB) && match.size() >= 5) {
            const int generatedLine = std::stoi(match[3].str());
            const std::string sev = match[1].str();
            entry.severity = (sev == "warning" || sev == "WARNING") ? LogSeverity::Warning : LogSeverity::Error;
            entry.line = RemapLineNumber(generatedLine, context);
            entry.message = match[4].str();
        } else if (std::regex_search(line, match, kPatternC) && match.size() >= 4) {
            const int generatedLine = std::stoi(match[1].str());
            const std::string sev = match[2].str();
            entry.severity = (sev == "warning" || sev == "WARNING") ? LogSeverity::Warning : LogSeverity::Error;
            entry.line = RemapLineNumber(generatedLine, context);
            entry.message = match[3].str();
        }

        entries.push_back(std::move(entry));
    }

    if (entries.empty() && !compilerLog.empty()) {
        CompileLogEntry fallback;
        fallback.severity = DetectSeverity(compilerLog);
        fallback.message = compilerLog;
        entries.push_back(std::move(fallback));
    }

    return entries;
}

std::string LineRemapper::MergeToText(const std::vector<CompileLogEntry>& entries) const
{
    if (entries.empty()) {
        return {};
    }

    std::ostringstream oss;
    for (const CompileLogEntry& entry : entries) {
        const char* severityText = "INFO";
        if (entry.severity == LogSeverity::Warning) {
            severityText = "WARNING";
        } else if (entry.severity == LogSeverity::Error) {
            severityText = "ERROR";
        }

        oss << severityText;
        if (entry.line > 0) {
            oss << " line " << entry.line;
        }
        oss << ": " << entry.message << '\n';
    }

    return oss.str();
}

std::vector<std::string> LineRemapper::SplitLines(const std::string& text)
{
    std::vector<std::string> lines;
    std::stringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

LogSeverity LineRemapper::DetectSeverity(const std::string& line)
{
    if (line.find("error") != std::string::npos || line.find("ERROR") != std::string::npos) {
        return LogSeverity::Error;
    }

    if (line.find("warning") != std::string::npos || line.find("WARNING") != std::string::npos) {
        return LogSeverity::Warning;
    }

    return LogSeverity::Info;
}

} // namespace shader
