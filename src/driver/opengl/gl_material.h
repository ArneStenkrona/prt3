#ifndef PRT3_GL_MATERIAL_H
#define PRT3_GL_MATERIAL_H

#include "src/driver/opengl/gl_shader.h"

#include <string>

namespace prt3 {

class GLMaterial {
public:
    GLMaterial(std::string const & vertex_shader_path,
               std::string const & fragment_shader_path);

    GLShader & shader() { return m_shader; }
private:
    GLShader m_shader;
};

}

#endif
