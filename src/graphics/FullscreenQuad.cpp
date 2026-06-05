#include "graphics/FullscreenQuad.h"

#include <array>

#include <glad/gl.h>

namespace graphics {

bool FullscreenQuad::Initialize()
{
    isInitialized_ = false;

    const std::array<float, 12> vertices{
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, -1.0f,
        1.0f, 1.0f,
        -1.0f, 1.0f
    };

    if (!vao_.Create()) {
        return false;
    }

    vao_.Bind();
    if (!vbo_.Create(GL_ARRAY_BUFFER, vertices.data(), sizeof(vertices), GL_STATIC_DRAW)) {
        VertexArray::Unbind();
        return false;
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    VertexBuffer::Unbind(GL_ARRAY_BUFFER);
    VertexArray::Unbind();

    isInitialized_ = true;
    return true;
}

void FullscreenQuad::Shutdown()
{
    isInitialized_ = false;
    vbo_ = VertexBuffer{};
    vao_ = VertexArray{};
}

void FullscreenQuad::Draw() const
{
    if (!isInitialized_) {
        return;
    }

    vao_.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    VertexArray::Unbind();
}

} // namespace graphics
