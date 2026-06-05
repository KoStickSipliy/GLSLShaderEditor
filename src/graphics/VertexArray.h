#pragma once

#include <glad/gl.h>

namespace graphics {

class VertexArray {
public:
    VertexArray() = default;
    ~VertexArray();

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& other) noexcept;
    VertexArray& operator=(VertexArray&& other) noexcept;

    bool Create();
    void Bind() const;
    static void Unbind();

    bool IsValid() const { return id_ != 0; }
    GLuint Id() const { return id_; }

private:
    void Reset();

private:
    GLuint id_ = 0;
};

} // namespace graphics
