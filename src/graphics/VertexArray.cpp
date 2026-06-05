#include "graphics/VertexArray.h"

namespace graphics {

VertexArray::~VertexArray()
{
    Reset();
}

VertexArray::VertexArray(VertexArray&& other) noexcept
{
    id_ = other.id_;
    other.id_ = 0;
}

VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
    if (this != &other) {
        Reset();
        id_ = other.id_;
        other.id_ = 0;
    }
    return *this;
}

bool VertexArray::Create()
{
    Reset();
    glGenVertexArrays(1, &id_);
    return id_ != 0;
}

void VertexArray::Bind() const
{
    glBindVertexArray(id_);
}

void VertexArray::Unbind()
{
    glBindVertexArray(0);
}

void VertexArray::Reset()
{
    if (id_ != 0) {
        glDeleteVertexArrays(1, &id_);
        id_ = 0;
    }
}

} // namespace graphics
