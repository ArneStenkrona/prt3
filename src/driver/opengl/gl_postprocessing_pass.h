#ifndef PRT3_GL_POSTPROCESSING_PASS_H
#define PRT3_GL_POSTPROCESSING_PASS_H

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>

namespace prt3 {

class GLPostProcessingPass {
public:
    GLPostProcessingPass();

    void set_shader(const char * fragment_shader_path);
    void render(int w, int h, GLuint render_texture);
private:
    GLuint m_screen_quad_vao;
    GLuint m_screen_quad_vbo;

    GLuint m_shader;
};

} // namespace prt3

#endif
