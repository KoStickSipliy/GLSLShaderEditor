#include "shader/ShaderCompilationService.h"

#include <chrono>

#include "graphics/GLResourceDiagnostics.h"

namespace {

const char* kBuiltInTestShader = R"(void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    float T = iTime * PARAM1 / 20.0;
    vec2 uv = fragCoord / iResolution.xy;
    vec3 col = 0.5 + 0.5 * cos(T + uv.xyx + vec3(0.0, 2.0, 4.0));
    fragColor = vec4(col, 1.0);
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
    const graphics::GLResourceSnapshot beforeSnapshot = graphics::GLResourceDiagnostics::Snapshot();

    report.generatedFragmentSource = preparedSource.generatedSource;
    report.sourceCharacterCount = preparedSource.userCharacterCount;

    std::string rawCompilerLog;
    const auto start = std::chrono::steady_clock::now();
    const bool success = renderer_.RebuildFragmentShader(preparedSource.generatedSource, rawCompilerLog);
    const auto end = std::chrono::steady_clock::now();

    report.success = success;
    report.durationMs = std::chrono::duration<double, std::milli>(end - start).count();
    report.entries = lineRemapper_.Remap(rawCompilerLog, preparedSource.remapContext);
    const graphics::GLResourceSnapshot afterSnapshot = graphics::GLResourceDiagnostics::Snapshot();
    if (!graphics::GLResourceDiagnostics::Matches(beforeSnapshot, afterSnapshot)) {
        CompileLogEntry entry;
        entry.severity = LogSeverity::Warning;
        entry.line = -1;
        entry.message = "Resource snapshot mismatch after compile: before(" +
            graphics::GLResourceDiagnostics::ToString(beforeSnapshot) + "), after(" +
            graphics::GLResourceDiagnostics::ToString(afterSnapshot) + ")";
        report.entries.push_back(entry);
    }
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
