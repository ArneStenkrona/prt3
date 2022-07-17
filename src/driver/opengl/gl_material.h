#ifndef PRT3_GL_MATERIAL_H
#define PRT3_GL_MATERIAL_H

#include "src/driver/opengl/gl_shader.h"

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
    // TODO: Retrieve an existing shader program
    //       instead of creating it here.
    //       This is currently very wasteful
    GLMaterial(GLShader & shader,
               GLuint albedo_map,
               GLuint normal_map,
               GLuint metallic_map,
               GLuint roughness_map,
               glm::vec4 albedo,
               float metallic,
               float roughness);

    GLShader & shader() { return m_shader; }
    GLuint & albedo_map() { return m_albedo_map; }
    GLuint & normal_map() { return m_normal_map; }
    GLuint & metallic_map() { return m_metallic_map; }
    GLuint & roughness_map() { return m_roughness_map; }
    GLuint & ambient_occlussion_map() { return m_ambient_occlusion_map; }

    glm::vec4 albedo() const { return m_albedo; }
    float metallic() const { return m_metallic; }
    float roughness() const { return m_roughness; }

private:
    GLShader m_shader;
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
