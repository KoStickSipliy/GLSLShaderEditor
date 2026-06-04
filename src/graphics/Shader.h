#pragma once

#include <string>

#include <glad/gl.h>

namespace graphics {

class Shader {
public:
    Shader() = default;
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    bool CompileFromSource(GLenum type, const std::string& source, std::string& outLog);

    GLuint Id() const { return id_; }

private:
    void Reset();

private:
    GLuint id_ = 0;
};

} // namespace graphics
