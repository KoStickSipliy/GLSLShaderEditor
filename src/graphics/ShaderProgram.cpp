#include "graphics/ShaderProgram.h"

#include <vector>

namespace graphics {

ShaderProgram::~ShaderProgram()
{
    Reset();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
{
    id_ = other.id_;
    other.id_ = 0;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    if (this != &other) {
        Reset();
        id_ = other.id_;
        other.id_ = 0;
    }
    return *this;
}

bool ShaderProgram::Link(const Shader& vertexShader, const Shader& fragmentShader, std::string& outLog)
{
    Reset();

    id_ = glCreateProgram();
    if (id_ == 0) {
        outLog = "glCreateProgram failed.";
        return false;
    }

    glAttachShader(id_, vertexShader.Id());
    glAttachShader(id_, fragmentShader.Id());
    glLinkProgram(id_);

    GLint linked = GL_FALSE;
    glGetProgramiv(id_, GL_LINK_STATUS, &linked);
    if (linked == GL_TRUE) {
        outLog.clear();
        return true;
    }

    GLint logLength = 0;
    glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> log(static_cast<std::size_t>(logLength > 0 ? logLength : 1));
    glGetProgramInfoLog(id_, static_cast<GLsizei>(log.size()), nullptr, log.data());
    outLog.assign(log.data());

    Reset();
    return false;
}

void ShaderProgram::Use() const
{
    glUseProgram(id_);
}

void ShaderProgram::SetFloat(const char* name, float value) const
{
    glUniform1f(GetUniformLocation(name), value);
}

void ShaderProgram::SetInt(const char* name, int value) const
{
    glUniform1i(GetUniformLocation(name), value);
}

void ShaderProgram::SetVec2(const char* name, float x, float y) const
{
    glUniform2f(GetUniformLocation(name), x, y);
}

void ShaderProgram::SetVec4(const char* name, float x, float y, float z, float w) const
{
    glUniform4f(GetUniformLocation(name), x, y, z, w);
}

GLint ShaderProgram::GetUniformLocation(const char* name) const
{
    return glGetUniformLocation(id_, name);
}

void ShaderProgram::Reset()
{
    if (id_ != 0) {
        glDeleteProgram(id_);
        id_ = 0;
    }
}

} // namespace graphics
