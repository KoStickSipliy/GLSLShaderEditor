#pragma once

#include <string>

#include "graphics/FullscreenQuad.h"
#include "graphics/Shader.h"
#include "graphics/ShaderProgram.h"

namespace graphics {

struct FullscreenQuadRenderState {
    float iTime = 0.0f;
    float iDeltaTime = 0.0f;
    int iFrame = 0;

    float iResolutionX = 0.0f;
    float iResolutionY = 0.0f;
    float iMouseX = 0.0f;
    float iMouseY = 0.0f;
    float iMouseZ = 0.0f;
    float iMouseW = 0.0f;

    float param1 = 50.0f;
    float param2 = 50.0f;
    float param3 = 50.0f;
};

class FullscreenQuadRenderer {
public:
    bool Initialize(const std::string& fragmentShaderSource, std::string& outLog);
    bool RebuildFragmentShader(const std::string& fragmentShaderSource, std::string& outLog);
    void Render(const FullscreenQuadRenderState& state) const;

    bool IsInitialized() const { return initialized_; }

private:
    bool BuildProgram(const std::string& fragmentShaderSource, Shader& outFragmentShader, ShaderProgram& outProgram, std::string& outLog) const;

private:
    Shader vertexShader_;
    Shader fragmentShader_;
    ShaderProgram shaderProgram_;
    FullscreenQuad fullscreenQuad_;
    bool initialized_ = false;
};

} // namespace graphics
