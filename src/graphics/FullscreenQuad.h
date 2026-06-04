#pragma once

#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"

namespace graphics {

class FullscreenQuad {
public:
    bool Initialize();
    void Draw() const;

private:
    VertexArray vao_;
    VertexBuffer vbo_;
};

} // namespace graphics
