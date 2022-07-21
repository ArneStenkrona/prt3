#ifndef PRT3_GL_MATERIAL_H
#define PRT3_GL_MATERIAL_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>

#include <string>

namespace prt3 {

class GLMaterial {
public:
    GLMaterial(GLuint & shader,
               GLuint albedo_map,
               GLuint normal_map,
               GLuint metallic_map,
               GLuint roughness_map,
               glm::vec4 albedo,
               float metallic,
               float roughness);

    GLuint const & shader() const { return m_shader; }
    GLuint albedo_map() const { return m_albedo_map; }
    GLuint normal_map() const { return m_normal_map; }
    GLuint metallic_map() const { return m_metallic_map; }
    GLuint roughness_map() const { return m_roughness_map; }
    GLuint ambient_occlussion_map() const { return m_ambient_occlusion_map; }

    glm::vec4 albedo() const { return m_albedo; }
    float metallic() const { return m_metallic; }
    float roughness() const { return m_roughness; }

private:
    GLuint m_shader;
    GLuint m_albedo_map;
    GLuint m_normal_map;
    GLuint m_metallic_map;
    GLuint m_roughness_map;
    GLuint m_ambient_occlusion_map;

    glm::vec4 m_albedo;
    float m_metallic;
    float m_roughness;
};

}

#endif
