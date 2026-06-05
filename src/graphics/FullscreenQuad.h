#pragma once

#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"

namespace graphics {

class FullscreenQuad {
public:
    bool Initialize();
    void Shutdown();
    void Draw() const;
    bool IsInitialized() const { return isInitialized_; }

private:
    VertexArray vao_;
    VertexBuffer vbo_;
    bool isInitialized_ = false;
};

} // namespace graphics
