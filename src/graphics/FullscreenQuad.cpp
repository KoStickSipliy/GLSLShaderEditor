#include "graphics/FullscreenQuad.h"

#include <array>

#include <glad/glad.h>

namespace graphics {

bool FullscreenQuad::Initialize()
{
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
        return false;
    }

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    return true;
}

void FullscreenQuad::Draw() const
{
    vao_.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

} // namespace graphics
