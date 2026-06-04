#pragma once

#include <glad/glad.h>

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

private:
    void Reset();

private:
    GLuint id_ = 0;
};

} // namespace graphics
