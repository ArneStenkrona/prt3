#ifndef PRT3_GL_MATERIAL_H
#define PRT3_GL_MATERIAL_H

#include "src/engine/rendering/material.h"
#include "src/driver/opengl/gl_shader.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <string>

namespace prt3 {

class GLMaterial {
public:
    GLMaterial(
        GLShader & shader,
        GLShader & animated_shader,
        GLuint albedo_map,
        GLuint normal_map,
        GLuint metallic_map,
        GLuint roughness_map,
        Material material
    );

    // TODO: something less hacky than storing
    //       two shaders
    GLShader const & get_shader(bool animated, bool /*transparent*/)
    const { return animated ? m_animated_shader : m_shader; }
    GLuint albedo_map()             const { return m_albedo_map; }
    GLuint normal_map()             const { return m_normal_map; }
    GLuint metallic_map()           const { return m_metallic_map; }
    GLuint roughness_map()          const { return m_roughness_map; }
    GLuint ambient_occlussion_map() const { return m_ambient_occlusion_map; }

    Material const & material() const { return m_material; }
    Material & material() { return m_material; }

private:
    GLShader m_shader;
    GLShader m_animated_shader;
    GLuint m_albedo_map;
    GLuint m_normal_map;
    GLuint m_metallic_map;
    GLuint m_roughness_map;
    GLuint m_ambient_occlusion_map;

    Material m_material;
};

}

#endif
