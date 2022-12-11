#include "gl_material.h"

using namespace prt3;

GLMaterial::GLMaterial(
  GLShader & shader,
  GLShader & animated_shader,
  GLuint albedo_map,
  GLuint normal_map,
  GLuint metallic_map,
  GLuint roughness_map,
  Material material
)
 : m_shader{shader},
   m_animated_shader{animated_shader},
   m_albedo_map{albedo_map},
   m_normal_map{normal_map},
   m_metallic_map{metallic_map},
   m_roughness_map{roughness_map},
   m_material{material} {
}
