#ifndef PRT3_POSTPROCESSING_H
#define PRT3_POSTPROCESSING_H

#include <string>

namespace prt3 {

struct PostProcessingPass {
    std::string fragment_shader_path = "assets/shaders/opengl/passthrough.fs";
    float downscale_factor = 1.0f;
};

} // namespace

#endif
