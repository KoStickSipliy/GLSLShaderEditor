#include "graphics/GLResourceDiagnostics.h"

#include <algorithm>
#include <sstream>

namespace {

graphics::GLResourceSnapshot gSnapshot;

void Increment(int& value)
{
    ++value;
}

void Decrement(int& value)
{
    value = (std::max)(0, value - 1);
}

} // namespace

namespace graphics {

void GLResourceDiagnostics::OnShaderCreated()
{
    Increment(gSnapshot.shaders);
}

void GLResourceDiagnostics::OnShaderDestroyed()
{
    Decrement(gSnapshot.shaders);
}

void GLResourceDiagnostics::OnProgramCreated()
{
    Increment(gSnapshot.programs);
}

void GLResourceDiagnostics::OnProgramDestroyed()
{
    Decrement(gSnapshot.programs);
}

void GLResourceDiagnostics::OnVertexArrayCreated()
{
    Increment(gSnapshot.vertexArrays);
}

void GLResourceDiagnostics::OnVertexArrayDestroyed()
{
    Decrement(gSnapshot.vertexArrays);
}

void GLResourceDiagnostics::OnVertexBufferCreated()
{
    Increment(gSnapshot.vertexBuffers);
}

void GLResourceDiagnostics::OnVertexBufferDestroyed()
{
    Decrement(gSnapshot.vertexBuffers);
}

GLResourceSnapshot GLResourceDiagnostics::Snapshot()
{
    return gSnapshot;
}

bool GLResourceDiagnostics::Matches(const GLResourceSnapshot& lhs, const GLResourceSnapshot& rhs)
{
    return lhs.shaders == rhs.shaders &&
        lhs.programs == rhs.programs &&
        lhs.vertexArrays == rhs.vertexArrays &&
        lhs.vertexBuffers == rhs.vertexBuffers;
}

std::string GLResourceDiagnostics::ToString(const GLResourceSnapshot& snapshot)
{
    std::ostringstream oss;
    oss << "shaders=" << snapshot.shaders
        << ", programs=" << snapshot.programs
        << ", vaos=" << snapshot.vertexArrays
        << ", vbos=" << snapshot.vertexBuffers;
    return oss.str();
}

} // namespace graphics
