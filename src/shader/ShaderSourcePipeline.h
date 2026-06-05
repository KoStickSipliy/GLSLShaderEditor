#pragma once

#include <string>

#include "shader/CompilationTypes.h"

namespace shader {

class ShaderSourcePipeline {
public:
    PreparedFragmentSource BuildFragmentSource(const std::string& userSource) const;

private:
    std::string Preprocess(const std::string& source) const;
    static int CountLines(const std::string& text);
};

} // namespace shader
