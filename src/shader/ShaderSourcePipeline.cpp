#include "shader/ShaderSourcePipeline.h"

#include <algorithm>

namespace {

const char* kInjectedPrefix = R"(#version 330 core
out vec4 FragColor;

uniform float iTime;
uniform float iDeltaTime;
uniform int   iFrame;
uniform vec2  iResolution;
uniform vec4  iMouse;
uniform float PARAM1;
uniform float PARAM2;
uniform float PARAM3;

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

} // namespace

namespace shader {

PreparedFragmentSource ShaderSourcePipeline::BuildFragmentSource(const std::string& userSource) const
{
    PreparedFragmentSource prepared;
    prepared.preprocessedUserSource = Preprocess(userSource);
    prepared.userCharacterCount = prepared.preprocessedUserSource.size();

    prepared.generatedSource.reserve(
        std::char_traits<char>::length(kInjectedPrefix) +
        prepared.preprocessedUserSource.size() +
        std::char_traits<char>::length(kInjectedWrapper));
    prepared.generatedSource += kInjectedPrefix;
    prepared.generatedSource += prepared.preprocessedUserSource;
    prepared.generatedSource += kInjectedWrapper;

    prepared.remapContext.usesLineDirective = true;
    prepared.remapContext.generatedUserStartLine = CountLines(kInjectedPrefix);
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
