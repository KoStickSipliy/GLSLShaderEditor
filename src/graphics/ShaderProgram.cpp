#include "graphics/ShaderProgram.h"

#include <utility>
#include <vector>

#include "graphics/GLResourceDiagnostics.h"

namespace graphics {

ShaderProgram::~ShaderProgram()
{
    Reset();
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
{
    id_ = other.id_;
    uniformLocationCache_ = std::move(other.uniformLocationCache_);
    other.id_ = 0;
    other.uniformLocationCache_.clear();
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
    if (this != &other) {
        Reset();
        id_ = other.id_;
        uniformLocationCache_ = std::move(other.uniformLocationCache_);
        other.id_ = 0;
        other.uniformLocationCache_.clear();
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
    GLResourceDiagnostics::OnProgramCreated();

    glAttachShader(id_, vertexShader.Id());
    glAttachShader(id_, fragmentShader.Id());
    glLinkProgram(id_);
    glDetachShader(id_, vertexShader.Id());
    glDetachShader(id_, fragmentShader.Id());

    GLint linked = GL_FALSE;
    glGetProgramiv(id_, GL_LINK_STATUS, &linked);
    if (linked == GL_TRUE) {
        uniformLocationCache_.clear();
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

bool ShaderProgram::Validate(std::string& outLog) const
{
    if (id_ == 0) {
        outLog = "Program is not created.";
        return false;
    }

    glValidateProgram(id_);

    GLint valid = GL_FALSE;
    glGetProgramiv(id_, GL_VALIDATE_STATUS, &valid);
    if (valid == GL_TRUE) {
        outLog.clear();
        return true;
    }

    GLint logLength = 0;
    glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> log(static_cast<std::size_t>(logLength > 0 ? logLength : 1));
    glGetProgramInfoLog(id_, static_cast<GLsizei>(log.size()), nullptr, log.data());
    outLog.assign(log.data());
    return false;
}

void ShaderProgram::Use() const
{
    glUseProgram(id_);
}

void ShaderProgram::Unuse()
{
    glUseProgram(0);
}

void ShaderProgram::SetFloat(const char* name, float value) const
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1f(location, value);
    }
}

void ShaderProgram::SetInt(const char* name, int value) const
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform1i(location, value);
    }
}

void ShaderProgram::SetVec2(const char* name, float x, float y) const
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform2f(location, x, y);
    }
}

void ShaderProgram::SetVec4(const char* name, float x, float y, float z, float w) const
{
    const GLint location = GetUniformLocation(name);
    if (location >= 0) {
        glUniform4f(location, x, y, z, w);
    }
}

GLint ShaderProgram::GetUniformLocation(const char* name) const
{
    const auto it = uniformLocationCache_.find(name);
    if (it != uniformLocationCache_.end()) {
        return it->second;
    }

    const GLint location = glGetUniformLocation(id_, name);
    uniformLocationCache_.emplace(name, location);
    return location;
}

void ShaderProgram::Reset()
{
    if (id_ != 0) {
        glDeleteProgram(id_);
        GLResourceDiagnostics::OnProgramDestroyed();
        id_ = 0;
    }
    uniformLocationCache_.clear();
}

} // namespace graphics
