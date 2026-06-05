#include "graphics/GLDebug.h"

#include <sstream>

namespace graphics {

bool InitializeGLDebugOutput()
{
    return true;
}

std::vector<GLDebugMessage> ConsumeGLDebugMessages()
{
    return {};
}

bool DrainOpenGLErrors(const char* stage, std::string& outMessage)
{
    GLenum error = glGetError();
    if (error == GL_NO_ERROR) {
        outMessage.clear();
        return false;
    }

    std::ostringstream oss;
    oss << stage << " OpenGL error(s):";
    while (error != GL_NO_ERROR) {
        oss << " 0x" << std::hex << std::uppercase << static_cast<unsigned int>(error);
        error = glGetError();
    }

    outMessage = oss.str();
    return true;
}

} // namespace graphics
