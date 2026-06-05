#pragma once

#include <string>
#include <vector>

#include <glad/gl.h>

namespace graphics {

enum class GLDebugSeverity {
    Info,
    Warning,
    Error
};

struct GLDebugMessage {
    GLDebugSeverity severity = GLDebugSeverity::Info;
    GLenum source = 0;
    GLenum type = 0;
    GLuint id = 0;
    std::string text;
};

bool InitializeGLDebugOutput();
std::vector<GLDebugMessage> ConsumeGLDebugMessages();
bool DrainOpenGLErrors(const char* stage, std::string& outMessage);

} // namespace graphics
