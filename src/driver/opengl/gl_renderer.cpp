#include "gl_renderer.h"

#include "src/driver/opengl/gl_shader_utility.h"
#include "src/driver/opengl/gl_utility.h"

#include "glm/gtx/string_cast.hpp"

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <vector>
#include <unordered_map>
#include <cassert>

using namespace prt3;

GLRenderer::GLRenderer(SDL_Window * window,
                       float downscale_factor)
 : m_window{window},
   m_downscale_factor{downscale_factor},
   m_material_manager{m_texture_manager},
   m_model_manager{m_material_manager}
  {
    EmscriptenWebGLContextAttributes attrs;
	attrs.antialias = true;
	attrs.majorVersion = 2;
	attrs.minorVersion = 0;
	attrs.alpha = false;
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context =
        emscripten_webgl_create_context("#canvas", &attrs);
	emscripten_webgl_make_context_current(webgl_context);

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

    m_texture_manager.init();
    m_material_manager.init();
}

GLRenderer::~GLRenderer() {
    // TODO: implement
}

void GLRenderer::set_postprocessing_chain(
    std::vector<PostProcessingPass> const & chain_info) {
    int w;
    int h;
    SDL_GetWindowSize(m_window, &w, &h);

    if (chain_info.empty()) {
        if (m_framebuffer != 0) {
            glDeleteFramebuffers(1, &m_framebuffer);
            glDeleteTextures(1, &m_color_texture);
            glDeleteTextures(1, &m_depth_texture);

            glDeleteFramebuffers(1, &m_normal_framebuffer);
            glDeleteTextures(1, &m_normal_texture);
            glDeleteTextures(1, &m_normal_depth_texture);
        }
        m_framebuffer = 0;
        m_color_texture = 0;
        m_normal_texture = 0;
        m_depth_texture = 0;

    } else {
        generate_framebuffer(m_framebuffer,
                             m_color_texture,
                             m_depth_texture);

    }
    generate_framebuffer(m_normal_framebuffer,
                         m_normal_texture,
                         m_normal_depth_texture);

    m_postprocessing_chain.set_chain(chain_info,
                                     m_color_texture,
                                     m_normal_texture,
                                     m_depth_texture,
                                     w, h);
}

void GLRenderer::render(RenderData const & render_data) {
    render_framebuffer(render_data,
                       m_framebuffer,
                       false);

    if (m_framebuffer != 0) {
        // m_framebuffer != 0 implies we have
        // post-processing, so we need to render
        // normal data
        render_framebuffer(render_data,
                           m_normal_framebuffer,
                           true);
    }

    m_postprocessing_chain.render(render_data.scene_data);

    SDL_GL_SwapWindow(m_window);
    glCheckError();
}

void GLRenderer::generate_framebuffer(GLuint & framebuffer,
                                      GLuint & render_texture,
                                      GLuint & depth_texture) {
        int w;
        int h;
        SDL_GetWindowSize(m_window, &w, &h);

        glGenFramebuffers(1, &framebuffer);
        glCheckError();
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glCheckError();

        glGenTextures(1, &render_texture);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, render_texture);
        glCheckError();
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB,
                     static_cast<GLint>(w / m_downscale_factor),
                     static_cast<GLint>(h / m_downscale_factor),
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glCheckError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glCheckError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glCheckError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glCheckError();

        glGenTextures(1, &depth_texture);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glCheckError();
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_DEPTH_COMPONENT24,
                     static_cast<GLint>(w / m_downscale_factor),
                     static_cast<GLint>(h / m_downscale_factor),
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
                               render_texture,
                               0);
        glCheckError();

        glFramebufferTexture2D(GL_FRAMEBUFFER,
                            GL_DEPTH_ATTACHMENT,
                            GL_TEXTURE_2D,
                            depth_texture,
                            0);
        glCheckError();

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            assert(false && "Failed to create framebuffer!");
        }
}

void GLRenderer::render_framebuffer(RenderData const & render_data,
                                    GLuint framebuffer,
                                    bool normal_pass) {
    // Bind the framebuffer
    int w;
    int h;
    SDL_GetWindowSize(m_window, &w, &h);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glCheckError();
    glViewport(0, 0,
               static_cast<GLint>(w / m_downscale_factor),
               static_cast<GLint>(h / m_downscale_factor));
    glCheckError();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();
    // Render meshes
    for (auto & pair : m_material_queues) {
        pair.second.resize(0);
    }
    for (MeshRenderData const & mesh_data : render_data.mesh_data) {
        m_material_queues[mesh_data.material_id].push_back(mesh_data);
    }

    std::vector<GLMaterial> const & materials = normal_pass ?
     m_material_manager.normal_materials() : m_material_manager.materials();
    for (auto const & pair : m_material_queues) {
        GLMaterial const & material = materials[pair.first];
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
}
