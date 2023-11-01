#ifndef PRT3_GL_POSTPROCESSING_CHAIN_H
#define PRT3_GL_POSTPROCESSING_CHAIN_H

#include "src/engine/rendering/postprocessing_chain.h"
#include "src/engine/rendering/postprocessing_pass.h"
#include "src/backend/opengl/gl_postprocessing_pass.h"
#include "src/backend/opengl/gl_source_buffers.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <vector>

namespace prt3 {

class GLPostProcessingChain {
public:
    ~GLPostProcessingChain() { clear_chain(); }

    void set_chain(
        PostProcessingChain const & chain,
        GLSourceBuffers const & source_buffers,
        int window_width,
        int window_height
    );

    void render(CameraRenderData const & camera_data, uint32_t frame);

    bool empty() const { return m_passes.empty(); }

    GLuint screen_quad_vao() const { return m_screen_quad_vao; };

private:
    std::vector<GLPostProcessingPass> m_passes;
    std::vector<GLuint> m_framebuffers;
    std::vector<GLuint> m_color_textures;

    GLuint m_screen_quad_vbo = 0;
    GLuint m_screen_quad_vao = 0;

    void clear_chain();
};

};

#endif
