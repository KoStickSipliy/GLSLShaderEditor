#pragma once

#include <string>

namespace graphics {

struct GLResourceSnapshot {
    int shaders = 0;
    int programs = 0;
    int vertexArrays = 0;
    int vertexBuffers = 0;
};

class GLResourceDiagnostics {
public:
    static void OnShaderCreated();
    static void OnShaderDestroyed();

    static void OnProgramCreated();
    static void OnProgramDestroyed();

    static void OnVertexArrayCreated();
    static void OnVertexArrayDestroyed();

    static void OnVertexBufferCreated();
    static void OnVertexBufferDestroyed();

    static GLResourceSnapshot Snapshot();
    static bool Matches(const GLResourceSnapshot& lhs, const GLResourceSnapshot& rhs);
    static std::string ToString(const GLResourceSnapshot& snapshot);
};

} // namespace graphics
