#include "gl_renderer.h"

#include "src/driver/opengl/gl_shader_utility.h"
#include "src/driver/opengl/gl_utility.h"

#include "glm/gtx/string_cast.hpp"

#include <vector>
#include <unordered_map>
#include <cassert>

using namespace prt3;

GLRenderer::GLRenderer(SDL_Window * window,
                       unsigned int scale_factor)
 : m_window{window},
   m_scale_factor{scale_factor},
   m_material_manager{m_texture_manager},
   m_model_manager{m_material_manager},
   m_postprocessing_pass{}
  {
    int w;
    int h;
    SDL_GetWindowSize(m_window, &w, &h);

    assert(w % static_cast<int>(m_scale_factor) != 0 && "Width not divisible by scale factor.");
    assert(h % static_cast<int>(m_scale_factor) != 0 && "Height not divisible by scale factor.");

    /* Set SDL attributes */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    /* Enable GL functionality */
    glEnable(GL_DEPTH_TEST);
    glCheckError();
    glDepthFunc(GL_LESS);
    glCheckError();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glCheckError();
    glEnable(GL_CULL_FACE);
    glCheckError();
    glCullFace(GL_BACK);
    glCheckError();

    /* Generate framebuffer */
    glGenFramebuffers(1, &m_framebuffer);
    glCheckError();
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glCheckError();

    glGenTextures(1, &m_render_texture);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, m_render_texture);
    glCheckError();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w / m_scale_factor, h / m_scale_factor, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glCheckError();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glCheckError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glCheckError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glCheckError();

	glGenTextures(1, &m_depth_texture);
    glCheckError();
	glBindTexture(GL_TEXTURE_2D, m_depth_texture);
    glCheckError();
	glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_DEPTH_COMPONENT,
                 w / m_scale_factor,
                 h / m_scale_factor,
                 0,
                 GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT,
                 0);
    glCheckError();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glCheckError();

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           m_render_texture,
                           0);
    glCheckError();

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D,
                           m_depth_texture,
                           0);
    glCheckError();

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        assert(false && "Failed to create framebuffer!");
    }
}

GLRenderer::~GLRenderer() {
    // TODO: implement
}

void GLRenderer::render(RenderData const & render_data) {
    // Bind the framebuffer
    int w;
 	int h;
    SDL_GetWindowSize(m_window, &w, &h);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glCheckError();
    glViewport(0, 0, w / m_scale_factor, h / m_scale_factor);
    glCheckError();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();
    std::unordered_map<ResourceID, std::vector<MeshRenderData>> m_material_queues;
    // Render meshes
    for (MeshRenderData const & mesh_data : render_data.mesh_data) {
        m_material_queues[mesh_data.material_id].push_back(mesh_data);
    }

    std::vector<GLMaterial> const & materials = m_material_manager.materials();
    for (auto const & pair : m_material_queues) {
        GLMaterial material = materials[pair.first];
        GLuint shader_id = material.shader();
        glUseProgram(shader_id);
        glCheckError();

        // Light data
        LightRenderData const & light_data = render_data.light_data;

        glUniform3fv(material.view_position_loc(), 1, &render_data.scene_data.view_position[0]);
        glUniform1i(material.number_of_point_lights(), static_cast<int>(light_data.number_of_point_lights));

        glUniform3fv(material.point_lights_0_position_loc(), 1, &light_data.point_lights[0].position[0]);
        glUniform3fv(material.point_lights_0_color_loc(), 1, &light_data.point_lights[0].light.color[0]);
        glUniform1f(material.point_lights_0_quadratic_loc(), light_data.point_lights[0].light.quadratic_term);
        glUniform1f(material.point_lights_0_linear_loc(), light_data.point_lights[0].light.linear_term);
        glUniform1f(material.point_lights_0_constant_loc(), light_data.point_lights[0].light.constant_term);

        glUniform3fv(material.point_lights_1_position_loc(), 1, &light_data.point_lights[1].position[0]);
        glUniform3fv(material.point_lights_1_color_loc(), 1, &light_data.point_lights[1].light.color[0]);
        glUniform1f(material.point_lights_1_quadratic_loc(), light_data.point_lights[1].light.quadratic_term);
        glUniform1f(material.point_lights_1_linear_loc(), light_data.point_lights[1].light.linear_term);
        glUniform1f(material.point_lights_1_constant_loc(), light_data.point_lights[1].light.constant_term);

        glUniform3fv(material.point_lights_2_position_loc(), 1, &light_data.point_lights[2].position[0]);
        glUniform3fv(material.point_lights_2_color_loc(), 1, &light_data.point_lights[2].light.color[0]);
        glUniform1f(material.point_lights_2_quadratic_loc(), light_data.point_lights[2].light.quadratic_term);
        glUniform1f(material.point_lights_2_linear_loc(), light_data.point_lights[2].light.linear_term);
        glUniform1f(material.point_lights_2_constant_loc(), light_data.point_lights[2].light.constant_term);

        glUniform3fv(material.point_lights_3_position_loc(), 1, &light_data.point_lights[3].position[0]);
        glUniform3fv(material.point_lights_3_color_loc(), 1, &light_data.point_lights[3].light.color[0]);
        glUniform1f(material.point_lights_3_quadratic_loc(), light_data.point_lights[3].light.quadratic_term);
        glUniform1f(material.point_lights_3_linear_loc(), light_data.point_lights[3].light.linear_term);
        glUniform1f(material.point_lights_3_constant_loc(), light_data.point_lights[3].light.constant_term);

        glm::vec3 dir_light_dir = glm::normalize(light_data.directional_light.direction);
        glUniform3fv(material.directional_light_direction_loc(), 1, &dir_light_dir[0]);
        glUniform3fv(material.directional_light_color_loc(), 1, &light_data.directional_light.color[0]);
        glUniform1i(material.directional_light_on_loc(), static_cast<int>(light_data.directional_light_on));

        glUniform3fv(material.ambient_light_loc(), 1, &light_data.ambient_light.color[0]);

        std::vector<GLMesh> const & meshes = m_model_manager.meshes();
        for (MeshRenderData const & mesh_data : pair.second) {
            meshes[mesh_data.mesh_id].draw(
                materials[mesh_data.material_id],
                render_data.scene_data,
                mesh_data
            );
        }
        glCheckError();
    }

    m_postprocessing_pass.render(w, h,
                                 render_data.scene_data,
                                 m_render_texture,
                                 m_depth_texture);

    SDL_GL_SwapWindow(m_window);
    glCheckError();
}

void GLRenderer::set_postprocessing_shader(const char * fragment_shader_path) {
    m_postprocessing_pass.set_shader(fragment_shader_path);
}
