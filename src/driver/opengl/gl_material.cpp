#include "gl_material.h"

using namespace prt3;

GLMaterial::GLMaterial(GLShader & shader,
                       GLuint albedo,
                       GLuint normal,
                       GLuint roughness)
 : m_shader{shader},
   m_albedo_map{albedo},
   m_normal_map{normal},
   m_roughness_map{roughness} {
}
