#pragma once

#include <cstddef>

#include <glad/glad.h>

namespace graphics {

class VertexBuffer {
public:
    VertexBuffer() = default;
    ~VertexBuffer();

    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    bool Create(GLenum target, const void* data, std::size_t sizeInBytes, GLenum usage);
    void Bind() const;

private:
    void Reset();

private:
    GLuint id_ = 0;
    GLenum target_ = GL_ARRAY_BUFFER;
};

} // namespace graphics
