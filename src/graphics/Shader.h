#pragma once

#include <string>
#include <string_view>

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

    bool CompileFromSource(GLenum type, std::string_view source, std::string& outLog);

    bool IsValid() const { return id_ != 0; }
    GLenum Type() const { return type_; }
    GLuint Id() const { return id_; }

private:
    void Reset();

private:
    GLuint id_ = 0;
    GLenum type_ = 0;
};

} // namespace graphics
