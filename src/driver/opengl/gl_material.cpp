#include "gl_material.h"

using namespace prt3;

GLMaterial::GLMaterial(GLuint & shader,
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
    set_attribute_cache();
}

void GLMaterial::set_attribute_cache() {
    /* Per-material attributes*/
    m_view_position_loc = glGetUniformLocation(m_shader, "u_ViewPosition");
    m_number_of_point_lights = glGetUniformLocation(m_shader, "u_NumberOfPointLights");
    m_point_lights_0_color_loc = glGetUniformLocation(m_shader, "u_PointLights[0].color");
    m_point_lights_0_position_loc = glGetUniformLocation(m_shader, "u_PointLights[0].position");
    m_point_lights_0_quadratic_loc = glGetUniformLocation(m_shader, "u_PointLights[0].a");
    m_point_lights_0_linear_loc = glGetUniformLocation(m_shader, "u_PointLights[0].b");
    m_point_lights_0_constant_loc = glGetUniformLocation(m_shader, "u_PointLights[0].c");
    m_point_lights_1_color_loc = glGetUniformLocation(m_shader, "u_PointLights[1].color");
    m_point_lights_1_position_loc = glGetUniformLocation(m_shader, "u_PointLights[1].position");
    m_point_lights_1_quadratic_loc = glGetUniformLocation(m_shader, "u_PointLights[1].a");
    m_point_lights_1_linear_loc = glGetUniformLocation(m_shader, "u_PointLights[1].b");
    m_point_lights_1_constant_loc = glGetUniformLocation(m_shader, "u_PointLights[1].c");
    m_point_lights_2_color_loc = glGetUniformLocation(m_shader, "u_PointLights[2].color");
    m_point_lights_2_position_loc = glGetUniformLocation(m_shader, "u_PointLights[2].position");
    m_point_lights_2_quadratic_loc = glGetUniformLocation(m_shader, "u_PointLights[2].a");
    m_point_lights_2_linear_loc = glGetUniformLocation(m_shader, "u_PointLights[2].b");
    m_point_lights_2_constant_loc = glGetUniformLocation(m_shader, "u_PointLights[2].c");
    m_point_lights_3_color_loc = glGetUniformLocation(m_shader, "u_PointLights[3].color");
    m_point_lights_3_position_loc = glGetUniformLocation(m_shader, "u_PointLights[3].position");
    m_point_lights_3_quadratic_loc = glGetUniformLocation(m_shader, "u_PointLights[3].a");
    m_point_lights_3_linear_loc = glGetUniformLocation(m_shader, "u_PointLights[3].b");
    m_point_lights_3_constant_loc = glGetUniformLocation(m_shader, "u_PointLights[3].c");
    m_directional_light_direction_loc = glGetUniformLocation(m_shader, "u_DirectionalLight.direction");
    m_directional_light_color_loc = glGetUniformLocation(m_shader, "u_DirectionalLight.color");
    m_directional_light_on_loc = glGetUniformLocation(m_shader, "u_DirectionalLightOn");
    m_ambient_light_loc = glGetUniformLocation(m_shader, "u_AmbientLight");

    /* Per-mesh attributes */
    m_mmatrix_loc = glGetUniformLocation(m_shader, "u_MMatrix");
    m_mvmatrix_loc = glGetUniformLocation(m_shader, "u_MVMatrix");
    m_mvpmatrix_loc = glGetUniformLocation(m_shader, "u_MVPMatrix");
    m_inv_tpos_matrix_loc = glGetUniformLocation(m_shader, "u_InvTposMMatrix");
    m_albedo_map_loc = glGetUniformLocation(m_shader, "u_AlbedoMap");
    m_normal_map_loc = glGetUniformLocation(m_shader, "u_NormalMap");
    m_metallic_map_loc = glGetUniformLocation(m_shader, "u_MetallicMap");
    m_roughness_map_loc = glGetUniformLocation(m_shader, "u_RoughnessMap");
    m_albedo_loc = glGetUniformLocation(m_shader, "u_Albedo");
    m_metallic_loc = glGetUniformLocation(m_shader, "u_Metallic");
    m_roughness_loc = glGetUniformLocation(m_shader, "u_Roughness");
}
