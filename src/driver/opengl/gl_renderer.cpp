#include "gl_renderer.h"

#include "src/driver/opengl/gl_shader_utility.h"
#include "src/driver/opengl/gl_utility.h"

#include <emscripten.h>

#include <vector>
#include <unordered_map>
#include <cassert>

using namespace prt3;

GLRenderer::GLRenderer(SDL_Window * window)
 : m_window{window},
   m_material_manager{m_texture_manager},
   m_model_manager{m_material_manager},
   m_postprocessing_pass{}
  {
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

    int w;
    int h;
    SDL_GetWindowSize(m_window, &w, &h);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 0);
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
    glViewport(0, 0, w, h);
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
        glshaderutility::set_vec3(shader_id, "u_ViewPosition", render_data.scene_data.view_position);
        glCheckError();
        // Light data
        LightRenderData const & light_data = render_data.light_data;
        glshaderutility::set_int(shader_id, "u_NumberOfPointLights", static_cast<int>(light_data.number_of_point_lights));

        glshaderutility::set_vec3(shader_id, "u_PointLights[0].color", light_data.point_lights[0].light.color);
        glshaderutility::set_vec3(shader_id, "u_PointLights[0].position", light_data.point_lights[0].position);
        glshaderutility::set_float(shader_id, "u_PointLights[0].a", light_data.point_lights[0].light.quadratic_term);
        glshaderutility::set_float(shader_id, "u_PointLights[0].b", light_data.point_lights[0].light.linear_term);
        glshaderutility::set_float(shader_id, "u_PointLights[0].c", light_data.point_lights[0].light.constant_term);

        glshaderutility::set_vec3(shader_id, "u_PointLights[1].position", light_data.point_lights[1].position);
        glshaderutility::set_vec3(shader_id, "u_PointLights[1].color", light_data.point_lights[1].light.color);
        glshaderutility::set_float(shader_id, "u_PointLights[1].a", light_data.point_lights[1].light.quadratic_term);
        glshaderutility::set_float(shader_id, "u_PointLights[1].b", light_data.point_lights[1].light.linear_term);
        glshaderutility::set_float(shader_id, "u_PointLights[1].c", light_data.point_lights[1].light.constant_term);

        glshaderutility::set_vec3(shader_id, "u_PointLights[2].position", light_data.point_lights[2].position);
        glshaderutility::set_vec3(shader_id, "u_PointLights[2].color", light_data.point_lights[2].light.color);
        glshaderutility::set_float(shader_id, "u_PointLights[2].a", light_data.point_lights[2].light.quadratic_term);
        glshaderutility::set_float(shader_id, "u_PointLights[2].b", light_data.point_lights[2].light.linear_term);
        glshaderutility::set_float(shader_id, "u_PointLights[2].c", light_data.point_lights[2].light.constant_term);

        glshaderutility::set_vec3(shader_id, "u_PointLights[3].position", light_data.point_lights[3].position);
        glshaderutility::set_vec3(shader_id, "u_PointLights[3].color", light_data.point_lights[3].light.color);
        glshaderutility::set_float(shader_id, "u_PointLights[3].a", light_data.point_lights[3].light.quadratic_term);
        glshaderutility::set_float(shader_id, "u_PointLights[3].b", light_data.point_lights[3].light.linear_term);
        glshaderutility::set_float(shader_id, "u_PointLights[3].c", light_data.point_lights[3].light.constant_term);
        glCheckError();

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
