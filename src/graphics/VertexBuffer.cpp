#include "graphics/VertexBuffer.h"

#include "graphics/GLResourceDiagnostics.h"

namespace graphics {

VertexBuffer::~VertexBuffer()
{
    Reset();
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
{
    id_ = other.id_;
    target_ = other.target_;
    other.id_ = 0;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
{
    if (this != &other) {
        Reset();
        id_ = other.id_;
        target_ = other.target_;
        other.id_ = 0;
    }
    return *this;
}

bool VertexBuffer::Create(GLenum target, const void* data, std::size_t sizeInBytes, GLenum usage)
{
    Reset();
    target_ = target;

    glGenBuffers(1, &id_);
    if (id_ == 0) {
        return false;
    }
    GLResourceDiagnostics::OnVertexBufferCreated();

    glBindBuffer(target_, id_);
    glBufferData(target_, static_cast<GLsizeiptr>(sizeInBytes), data, usage);
    return true;
}

void VertexBuffer::Bind() const
{
    glBindBuffer(target_, id_);
}

void VertexBuffer::SetData(const void* data, std::size_t sizeInBytes, GLenum usage) const
{
    glBindBuffer(target_, id_);
    glBufferData(target_, static_cast<GLsizeiptr>(sizeInBytes), data, usage);
}

void VertexBuffer::Unbind(GLenum target)
{
    glBindBuffer(target, 0);
}

void VertexBuffer::Reset()
{
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
        GLResourceDiagnostics::OnVertexBufferDestroyed();
        id_ = 0;
    }
}

} // namespace graphics
