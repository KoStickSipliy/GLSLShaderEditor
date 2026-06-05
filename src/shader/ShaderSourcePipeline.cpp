#include "shader/ShaderSourcePipeline.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace {

const char* kInjectedHeaderPrefix = R"(#version 330 core
out vec4 FragColor;

uniform float iTime;
uniform float iDeltaTime;
uniform int   iFrame;
uniform vec2  iResolution;
uniform vec4  iMouse;
uniform float u_PARAM1;
uniform float u_PARAM2;
uniform float u_PARAM3;
)";

const char* kLineDirectiveToUser = R"(
#line 1
)";

const char* kInjectedWrapper = R"(
#line 100000
void main()
{
    vec4 fragColor = vec4(0.0);
    vec2 fragCoord = gl_FragCoord.xy;
    mainImage(fragColor, fragCoord);
    FragColor = fragColor;
}
)";

bool HasDefine(const std::string& source, const char* symbol)
{
    std::istringstream stream(source);
    std::string line;
    const std::size_t symbolLength = std::char_traits<char>::length(symbol);

    while (std::getline(stream, line)) {
        std::size_t pos = line.find_first_not_of(" \t");
        if (pos == std::string::npos || line[pos] != '#') {
            continue;
        }

        ++pos;
        pos = line.find_first_not_of(" \t", pos);
        if (pos == std::string::npos || line.compare(pos, 6, "define") != 0) {
            continue;
        }

        pos += 6;
        if (pos < line.size() && !std::isspace(static_cast<unsigned char>(line[pos]))) {
            continue;
        }

        pos = line.find_first_not_of(" \t", pos);
        if (pos == std::string::npos) {
            continue;
        }

        if (line.compare(pos, symbolLength, symbol) != 0) {
            continue;
        }

        const std::size_t end = pos + symbolLength;
        if (end >= line.size() ||
            std::isspace(static_cast<unsigned char>(line[end])) ||
            line[end] == '(') {
            return true;
        }
    }

    return false;
}

void AppendMissingParamDefine(std::string& prefix, const std::string& userSource, const char* symbol, const char* replacement)
{
    if (HasDefine(userSource, symbol)) {
        return;
    }

    prefix += "#ifndef ";
    prefix += symbol;
    prefix += "\n#define ";
    prefix += symbol;
    prefix += " (";
    prefix += replacement;
    prefix += ")\n#endif\n\n";
}

} // namespace

namespace shader {

PreparedFragmentSource ShaderSourcePipeline::BuildFragmentSource(const std::string& userSource) const
{
    PreparedFragmentSource prepared;
    prepared.preprocessedUserSource = Preprocess(userSource);
    prepared.userCharacterCount = prepared.preprocessedUserSource.size();

    std::string injectedPrefix = kInjectedHeaderPrefix;
    AppendMissingParamDefine(injectedPrefix, prepared.preprocessedUserSource, "PARAM1", "u_PARAM1");
    AppendMissingParamDefine(injectedPrefix, prepared.preprocessedUserSource, "PARAM2", "u_PARAM2");
    AppendMissingParamDefine(injectedPrefix, prepared.preprocessedUserSource, "PARAM3", "u_PARAM3");
    injectedPrefix += kLineDirectiveToUser;

    prepared.generatedSource.reserve(
        injectedPrefix.size() +
        prepared.preprocessedUserSource.size() +
        std::char_traits<char>::length(kInjectedWrapper));
    prepared.generatedSource += injectedPrefix;
    prepared.generatedSource += prepared.preprocessedUserSource;
    prepared.generatedSource += kInjectedWrapper;

    prepared.remapContext.usesLineDirective = true;
    prepared.remapContext.generatedUserStartLine = CountLines(injectedPrefix);
    prepared.remapContext.generatedWrapperStartLine =
        prepared.remapContext.generatedUserStartLine + CountLines(prepared.preprocessedUserSource);
    return prepared;
}

std::string ShaderSourcePipeline::Preprocess(const std::string& source) const
{
    std::string normalized = source;

    normalized.erase(std::remove(normalized.begin(), normalized.end(), '\r'), normalized.end());

    if (normalized.size() >= 3 &&
        static_cast<unsigned char>(normalized[0]) == 0xEF &&
        static_cast<unsigned char>(normalized[1]) == 0xBB &&
        static_cast<unsigned char>(normalized[2]) == 0xBF) {
        normalized.erase(0, 3);
    }

    if (!normalized.empty() && normalized.back() != '\n') {
        normalized.push_back('\n');
    }

    return normalized;
}

int ShaderSourcePipeline::CountLines(const std::string& text)
{
    if (text.empty()) {
        return 1;
    }

    int lines = 1;
    for (const char c : text) {
        if (c == '\n') {
            ++lines;
        }
    }
    return lines;
}

} // namespace shader
