#include "gl_material.h"

using namespace prt3;

GLMaterial::GLMaterial(GLShader & shader,
                       GLuint albedo_map,
                       GLuint normal_map,
                       GLuint metallic_map,
                       GLuint roughness_map,
                       glm::vec4 albedo,
                       float metallic,
                       float roughness)
 : m_shader{shader},
   m_albedo_map{albedo_map},
   m_normal_map{normal_map},
   m_metallic_map{metallic_map},
   m_roughness_map{roughness_map},
   m_albedo{albedo},
   m_metallic{metallic},
   m_roughness{roughness} {
}
