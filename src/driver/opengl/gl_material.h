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
    GLMaterial(GLShader & shader,
               GLuint albedo,
               GLuint normal,
               GLuint roughness);

    GLShader & shader() { return m_shader; }
    GLuint & albedo_map() { return m_albedo_map; }
    GLuint & normal_map() { return m_normal_map; }
    GLuint & roughness_map() { return m_roughness_map; }
private:
    GLShader m_shader;
    GLuint m_albedo_map;
    GLuint m_normal_map;
    GLuint m_roughness_map;
};

}

#endif
