#ifndef PRT3_GL_POSTPROCESSING_PASS_H
#define PRT3_GL_POSTPROCESSING_PASS_H

#include "src/engine/rendering/render_data.h"
#include "src/driver/opengl/gl_source_buffers.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

namespace prt3 {

class GLPostProcessingPass {
public:
    GLPostProcessingPass(
        const char * fragment_shader_path,
        unsigned int width,
        unsigned int height,
        GLSourceBuffers const & source_buffer,
        GLuint previous_color_buffer,
        GLuint target_framebuffer
    );

    void render(CameraRenderData const & camera_data);
private:
    GLuint m_screen_quad_vao;
    GLuint m_screen_quad_vbo;

    GLuint m_shader;

    unsigned int m_width;
    unsigned int m_height;

    GLSourceBuffers const * m_source_buffer;
    GLuint m_previous_color_buffer;
    GLuint m_target_framebuffer;

    void set_shader(const char * fragment_shader_path);
};

} // namespace prt3

#endif
