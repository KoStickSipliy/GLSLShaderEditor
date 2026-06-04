#pragma once

#include <string>

#include <glad/glad.h>

#include "graphics/Shader.h"

namespace graphics {

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    bool Link(const Shader& vertexShader, const Shader& fragmentShader, std::string& outLog);

    void Use() const;

    void SetFloat(const char* name, float value) const;
    void SetInt(const char* name, int value) const;
    void SetVec2(const char* name, float x, float y) const;
    void SetVec4(const char* name, float x, float y, float z, float w) const;

private:
    GLint GetUniformLocation(const char* name) const;
    void Reset();

private:
    GLuint id_ = 0;
};

} // namespace graphics
