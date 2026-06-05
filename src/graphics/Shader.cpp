#include "graphics/Shader.h"

#include <vector>

namespace graphics {

Shader::~Shader()
{
    Reset();
}

Shader::Shader(Shader&& other) noexcept
{
    id_ = other.id_;
    type_ = other.type_;
    other.id_ = 0;
    other.type_ = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other) {
        Reset();
        id_ = other.id_;
        type_ = other.type_;
        other.id_ = 0;
        other.type_ = 0;
    }
    return *this;
}

bool Shader::CompileFromSource(GLenum type, std::string_view source, std::string& outLog)
{
    Reset();
    type_ = type;

    id_ = glCreateShader(type);
    if (id_ == 0) {
        outLog = "glCreateShader failed.";
        return false;
    }

    const char* sourcePtr = source.data();
    const GLint sourceLength = static_cast<GLint>(source.size());
    glShaderSource(id_, 1, &sourcePtr, &sourceLength);
    glCompileShader(id_);

    GLint compiled = GL_FALSE;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_TRUE) {
        outLog.clear();
        return true;
    }

    GLint logLength = 0;
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &logLength);
    std::vector<char> log(static_cast<std::size_t>(logLength > 0 ? logLength : 1));
    glGetShaderInfoLog(id_, static_cast<GLsizei>(log.size()), nullptr, log.data());
    outLog.assign(log.data());

    Reset();
    return false;
}

void Shader::Reset()
{
    if (id_ != 0) {
        glDeleteShader(id_);
        id_ = 0;
    }
    type_ = 0;
}

} // namespace graphics
