#include "graphics/FullscreenQuadRenderer.h"

#include <utility>

namespace {

const char* kFullscreenVertexShaderSource = R"(#version 330 core
layout(location = 0) in vec2 aPosition;

void main()
{
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)";

} // namespace

namespace graphics {

bool FullscreenQuadRenderer::Initialize(const std::string& fragmentShaderSource, std::string& outLog)
{
    initialized_ = false;

    if (!vertexShader_.CompileFromSource(GL_VERTEX_SHADER, kFullscreenVertexShaderSource, outLog)) {
        return false;
    }

    if (!fullscreenQuad_.Initialize()) {
        outLog = "Failed to initialize fullscreen quad geometry.";
        return false;
    }

    if (!BuildProgram(fragmentShaderSource, fragmentShader_, shaderProgram_, outLog)) {
        return false;
    }

    initialized_ = true;
    return true;
}

bool FullscreenQuadRenderer::RebuildFragmentShader(const std::string& fragmentShaderSource, std::string& outLog)
{
    if (!vertexShader_.IsValid()) {
        outLog = "Renderer is not initialized.";
        return false;
    }

    Shader candidateFragmentShader;
    ShaderProgram candidateProgram;
    if (!BuildProgram(fragmentShaderSource, candidateFragmentShader, candidateProgram, outLog)) {
        return false;
    }

    fragmentShader_ = std::move(candidateFragmentShader);
    shaderProgram_ = std::move(candidateProgram);
    initialized_ = true;
    return true;
}

void FullscreenQuadRenderer::Shutdown()
{
    initialized_ = false;
    shaderProgram_ = ShaderProgram{};
    fragmentShader_ = Shader{};
    vertexShader_ = Shader{};
    fullscreenQuad_.Shutdown();
}

void FullscreenQuadRenderer::Render(const FullscreenQuadRenderState& state) const
{
    if (!initialized_) {
        return;
    }

    shaderProgram_.Use();
    shaderProgram_.SetFloat("iTime", state.iTime);
    shaderProgram_.SetFloat("iDeltaTime", state.iDeltaTime);
    shaderProgram_.SetInt("iFrame", state.iFrame);
    shaderProgram_.SetVec2("iResolution", state.iResolutionX, state.iResolutionY);
    shaderProgram_.SetVec4("iMouse", state.iMouseX, state.iMouseY, state.iMouseZ, state.iMouseW);
    shaderProgram_.SetVec2("uViewportOrigin", state.viewportOriginX, state.viewportOriginY);
    shaderProgram_.SetFloat("u_PARAM1", state.param1);
    shaderProgram_.SetFloat("u_PARAM2", state.param2);
    shaderProgram_.SetFloat("u_PARAM3", state.param3);
    fullscreenQuad_.Draw();
    ShaderProgram::Unuse();
}

bool FullscreenQuadRenderer::BuildProgram(const std::string& fragmentShaderSource, Shader& outFragmentShader, ShaderProgram& outProgram, std::string& outLog) const
{
    if (!outFragmentShader.CompileFromSource(GL_FRAGMENT_SHADER, fragmentShaderSource, outLog)) {
        return false;
    }

    if (!outProgram.Link(vertexShader_, outFragmentShader, outLog)) {
        return false;
    }

    outProgram.Use();
    if (!outProgram.Validate(outLog)) {
        ShaderProgram::Unuse();
        return false;
    }
    ShaderProgram::Unuse();

    outLog.clear();
    return true;
}

} // namespace graphics
