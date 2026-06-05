#include "shader/ShaderCompilationService.h"

#include <chrono>

namespace {

const char* kBuiltInTestShader = R"(void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = fragCoord / iResolution.xy;
    float t = iTime * 0.75;
    vec3 grad = vec3(uv.x, uv.y, 0.5 + 0.5 * sin(t));
    vec3 waves = 0.25 * cos(t + uv.xyx * 8.0 + vec3(0.0, 2.0, 4.0));
    fragColor = vec4(grad + waves, 1.0);
}
)";

} // namespace

namespace shader {

ShaderCompilationService::ShaderCompilationService(graphics::FullscreenQuadRenderer& renderer)
    : renderer_(renderer)
{
}

const char* ShaderCompilationService::BuiltInTestShader() const
{
    return kBuiltInTestShader;
}

CompileReport ShaderCompilationService::CompileAndHotSwap(const std::string& userSource) const
{
    CompileReport report;
    const PreparedFragmentSource preparedSource = sourcePipeline_.BuildFragmentSource(userSource);

    report.generatedFragmentSource = preparedSource.generatedSource;
    report.sourceCharacterCount = preparedSource.userCharacterCount;

    std::string rawCompilerLog;
    const auto start = std::chrono::steady_clock::now();
    const bool success = renderer_.RebuildFragmentShader(preparedSource.generatedSource, rawCompilerLog);
    const auto end = std::chrono::steady_clock::now();

    report.success = success;
    report.durationMs = std::chrono::duration<double, std::milli>(end - start).count();
    report.entries = lineRemapper_.Remap(rawCompilerLog, preparedSource.remapContext);
    report.mergedLogText = lineRemapper_.MergeToText(report.entries);
    report.statusText = success ? "Compiled" : "Compile failed";

    if (success && report.mergedLogText.empty()) {
        report.mergedLogText = "Compilation succeeded.";
    }

    return report;
}

PreparedFragmentSource ShaderCompilationService::BuildBuiltInPreparedSource() const
{
    return sourcePipeline_.BuildFragmentSource(kBuiltInTestShader);
}

} // namespace shader
