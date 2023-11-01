#ifndef PRT3_GL_POSTPROCESSING_PASS_H
#define PRT3_GL_POSTPROCESSING_PASS_H

#include "src/engine/rendering/render_data.h"
#include "src/backend/opengl/gl_source_buffers.h"
#include "src/backend/opengl/gl_shader.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

namespace prt3 {

class GLPostProcessingPass {
public:
    GLPostProcessingPass(
        GLuint shader,
        unsigned int width,
        unsigned int height,
        GLSourceBuffers const & source_buffer,
        GLuint previous_color_buffer,
        GLuint target_framebuffer
    );

    void render(
        CameraRenderData const & camera_data,
        uint32_t frame,
        GLuint screen_quad_vao
    );

    GLShader const & shader() const { return m_shader; }

private:

    GLShader m_shader;

    unsigned int m_width;
    unsigned int m_height;

    GLSourceBuffers const * m_source_buffer;
    GLuint m_previous_color_buffer;
    GLuint m_target_framebuffer;
};

} // namespace prt3

#endif
