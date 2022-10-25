#ifndef PRT3_GL_MATERIAL_H
#define PRT3_GL_MATERIAL_H

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
    GLMaterial(GLuint & shader,
               GLuint albedo_map,
               GLuint normal_map,
               GLuint metallic_map,
               GLuint roughness_map,
               glm::vec4 albedo,
               float metallic,
               float roughness);

    GLuint const & shader()         const { return m_shader; }
    GLuint albedo_map()             const { return m_albedo_map; }
    GLuint normal_map()             const { return m_normal_map; }
    GLuint metallic_map()           const { return m_metallic_map; }
    GLuint roughness_map()          const { return m_roughness_map; }
    GLuint ambient_occlussion_map() const { return m_ambient_occlusion_map; }

    glm::vec4 albedo() const { return m_albedo; }
    float metallic()   const { return m_metallic; }
    float roughness()  const { return m_roughness; }

    /* Per-material attribute locations */
    GLint view_position_loc()               const { return m_view_position_loc; }
    GLint number_of_point_lights()          const { return m_number_of_point_lights; }
    GLint point_lights_0_color_loc()        const { return m_point_lights_0_color_loc; }
    GLint point_lights_0_position_loc()     const { return m_point_lights_0_position_loc; }
    GLint point_lights_0_quadratic_loc()    const { return m_point_lights_0_quadratic_loc; }
    GLint point_lights_0_linear_loc()       const { return m_point_lights_0_linear_loc; }
    GLint point_lights_0_constant_loc()     const { return m_point_lights_0_constant_loc; }
    GLint point_lights_1_color_loc()        const { return m_point_lights_1_color_loc; }
    GLint point_lights_1_position_loc()     const { return m_point_lights_1_position_loc; }
    GLint point_lights_1_quadratic_loc()    const { return m_point_lights_1_quadratic_loc; }
    GLint point_lights_1_linear_loc()       const { return m_point_lights_1_linear_loc; }
    GLint point_lights_1_constant_loc()     const { return m_point_lights_1_constant_loc; }
    GLint point_lights_2_color_loc()        const { return m_point_lights_2_color_loc; }
    GLint point_lights_2_position_loc()     const { return m_point_lights_2_position_loc; }
    GLint point_lights_2_quadratic_loc()    const { return m_point_lights_2_quadratic_loc; }
    GLint point_lights_2_linear_loc()       const { return m_point_lights_2_linear_loc; }
    GLint point_lights_2_constant_loc()     const { return m_point_lights_2_constant_loc; }
    GLint point_lights_3_color_loc()        const { return m_point_lights_3_color_loc; }
    GLint point_lights_3_position_loc()     const { return m_point_lights_3_position_loc; }
    GLint point_lights_3_quadratic_loc()    const { return m_point_lights_3_quadratic_loc; }
    GLint point_lights_3_linear_loc()       const { return m_point_lights_3_linear_loc; }
    GLint point_lights_3_constant_loc()     const { return m_point_lights_3_constant_loc; }
    GLint directional_light_direction_loc() const { return m_directional_light_direction_loc; }
    GLint directional_light_color_loc()     const { return m_directional_light_color_loc; }
    GLint directional_light_on_loc()        const { return m_directional_light_on_loc; }
    GLint ambient_light_loc()               const { return m_ambient_light_loc; }

    /* Per-mesh attribute locations */
    GLint mmatrix_loc()         const { return m_mmatrix_loc; }
    GLint mvmatrix_loc()        const { return m_mvmatrix_loc; }
    GLint mvpmatrix_loc()       const { return m_mvpmatrix_loc; }
    GLint inv_tpos_matrix_loc() const { return m_inv_tpos_matrix_loc; }
    GLint albedo_map_loc()      const { return m_albedo_map_loc; }
    GLint normal_map_loc()      const { return m_normal_map_loc; }
    GLint metallic_map_loc()    const { return m_metallic_map_loc; }
    GLint roughness_map_loc()   const { return m_roughness_map_loc; }
    GLint albedo_loc()          const { return m_albedo_loc; }
    GLint metallic_loc()        const { return m_metallic_loc; }
    GLint roughness_loc()       const { return m_roughness_loc; }
    GLint input_value_loc()         const { return m_input_value_loc; }

private:
    GLuint m_shader;
    GLuint m_albedo_map;
    GLuint m_normal_map;
    GLuint m_metallic_map;
    GLuint m_roughness_map;
    GLuint m_ambient_occlusion_map;

    GLint m_view_position_loc;
    GLint m_number_of_point_lights;
    GLint m_point_lights_0_color_loc;
    GLint m_point_lights_0_position_loc;
    GLint m_point_lights_0_quadratic_loc;
    GLint m_point_lights_0_linear_loc;
    GLint m_point_lights_0_constant_loc;
    GLint m_point_lights_1_color_loc;
    GLint m_point_lights_1_position_loc;
    GLint m_point_lights_1_quadratic_loc;
    GLint m_point_lights_1_linear_loc;
    GLint m_point_lights_1_constant_loc;
    GLint m_point_lights_2_color_loc;
    GLint m_point_lights_2_position_loc;
    GLint m_point_lights_2_quadratic_loc;
    GLint m_point_lights_2_linear_loc;
    GLint m_point_lights_2_constant_loc;
    GLint m_point_lights_3_color_loc;
    GLint m_point_lights_3_position_loc;
    GLint m_point_lights_3_quadratic_loc;
    GLint m_point_lights_3_linear_loc;
    GLint m_point_lights_3_constant_loc;
    GLint m_directional_light_direction_loc;
    GLint m_directional_light_color_loc;
    GLint m_directional_light_on_loc;
    GLint m_ambient_light_loc;

    GLint m_mmatrix_loc;
    GLint m_mvmatrix_loc;
    GLint m_mvpmatrix_loc;
    GLint m_inv_tpos_matrix_loc;
    GLint m_albedo_map_loc;
    GLint m_normal_map_loc;
    GLint m_metallic_map_loc;
    GLint m_roughness_map_loc;
    GLint m_albedo_loc;
    GLint m_metallic_loc;
    GLint m_roughness_loc;
    GLint m_input_value_loc;

    glm::vec4 m_albedo;
    float m_metallic;
    float m_roughness;

    void set_attribute_cache();
};

}

#endif
