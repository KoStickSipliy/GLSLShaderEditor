#pragma once

#include <string>

#include "graphics/FullscreenQuadRenderer.h"
#include "shader/CompilationTypes.h"
#include "shader/LineRemapper.h"
#include "shader/ShaderSourcePipeline.h"

namespace shader {

class ShaderCompilationService {
public:
    explicit ShaderCompilationService(graphics::FullscreenQuadRenderer& renderer);

    const char* BuiltInTestShader() const;
    CompileReport CompileAndHotSwap(const std::string& userSource) const;
    PreparedFragmentSource BuildBuiltInPreparedSource() const;

private:
    graphics::FullscreenQuadRenderer& renderer_;
    ShaderSourcePipeline sourcePipeline_;
    LineRemapper lineRemapper_;
};

} // namespace shader
