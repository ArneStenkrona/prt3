#ifndef PRT3_GL_MATERIAL_H
#define PRT3_GL_MATERIAL_H

#include "src/driver/opengl/gl_shader.h"

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>

#include <string>

namespace prt3 {

class GLMaterial {
public:
    // TODO: Retrieve an existing shader program
    //       instead of creating it here.
    //       This is currently very wasteful
    GLMaterial(char const * vertex_shader_path,
               char const * fragment_shader_path,
               char const * albedo_path);

    GLShader & shader() { return m_shader; }
    GLuint & texture() { return m_texture; }
private:
    GLShader m_shader;
    GLuint m_texture;
};

}

#endif
