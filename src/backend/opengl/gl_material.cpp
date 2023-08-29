#include "gl_material.h"

using namespace prt3;

GLMaterial::GLMaterial(
    GLShader & shader,
    GLShader & animated_shader,
    GLShader & transparent_shader,
    GLShader & transparent_animated_shader,
    GLuint albedo_map,
    GLuint normal_map,
    GLuint metallic_map,
    GLuint roughness_map,
    Material material
)
 : m_shader{&shader},
   m_animated_shader{&animated_shader},
   m_transparent_shader{&transparent_shader},
   m_transparent_animated_shader{&transparent_animated_shader},
   m_albedo_map{albedo_map},
   m_normal_map{normal_map},
   m_metallic_map{metallic_map},
   m_roughness_map{roughness_map},
   m_material{material} {
}

GLShader const & GLMaterial::get_shader(
    bool animated,
    bool transparent
) const {
    if (transparent) {
        return animated ? *m_transparent_animated_shader : *m_transparent_shader;
    } else {
        return animated ? *m_animated_shader : *m_shader;
    }
}
