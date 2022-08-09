#ifndef PRT3_GL_POSTPROCESSING_PASS_H
#define PRT3_GL_POSTPROCESSING_PASS_H

#include "src/engine/rendering/render_data.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

namespace prt3 {

class GLPostProcessingPass {
public:
    GLPostProcessingPass(const char * fragment_shader_path,
                         unsigned int width,
                         unsigned int height,
                         GLuint source_color_texture,
                         GLuint source_normal_texture,
                         GLuint source_depth_texture,
                         GLuint target_framebuffer);

    void render(SceneRenderData const & scene_data);
private:
    GLuint m_screen_quad_vao;
    GLuint m_screen_quad_vbo;

    GLuint m_shader;

    unsigned int m_width;
    unsigned int m_height;

    GLuint m_source_color_texture;
    GLuint m_source_normal_texture;
    GLuint m_source_depth_texture;
    GLuint m_target_framebuffer;

    void set_shader(const char * fragment_shader_path);
};

} // namespace prt3

#endif
