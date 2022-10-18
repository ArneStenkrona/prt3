#ifndef PRT3_GL_POSTPROCESSING_CHAIN_H
#define PRT3_GL_POSTPROCESSING_CHAIN_H

#include "src/engine/rendering/postprocessing_pass.h"
#include "src/driver/opengl/gl_postprocessing_pass.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <vector>

namespace prt3 {

class GLPostProcessingChain {
public:
    void set_chain(std::vector<PostProcessingPass> const & chain_info,
                   GLuint source_color_texture,
                   GLuint source_normal_texture,
                   GLuint source_depth_texture,
                   int window_width,
                   int window_height);

    void render(CameraRenderData const & camera_data);
private:
    std::vector<GLPostProcessingPass> m_passes;
    std::vector<GLuint> m_framebuffers;
    std::vector<GLuint> m_color_textures;

    void clear_chain();
};

};

#endif
