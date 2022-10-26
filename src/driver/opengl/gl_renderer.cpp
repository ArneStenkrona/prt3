#include "gl_renderer.h"

#include "src/driver/opengl/gl_shader_utility.h"
#include "src/driver/opengl/gl_utility.h"

#include "glm/gtx/string_cast.hpp"

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <vector>
#include <unordered_map>
#include <cassert>
#include <iostream>

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
    attrs.enableExtensionsByDefault = true;
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context =
        emscripten_webgl_create_context("#canvas", &attrs);

	if (emscripten_webgl_make_context_current(webgl_context) !=
        EMSCRIPTEN_RESULT_SUCCESS) {
        std::cout << "Failed to make context current." << std::endl;
    }


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

    ImGui_ImplSDL2_InitForOpenGL(window, SDL_GL_GetCurrentContext());
    ImGui_ImplOpenGL3_Init();

    int w;
    int h;
    SDL_GetWindowSize(m_window, &w, &h);
    GLint buffer_width = static_cast<GLint>(w / m_downscale_factor);
    GLint buffer_height = static_cast<GLint>(h / m_downscale_factor);
    m_source_buffers.init(buffer_width, buffer_height);
}

GLRenderer::~GLRenderer() {
    // TODO: implement
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void GLRenderer::set_postprocessing_chain(
    PostProcessingChain const & chain) {
    int w;
    int h;
    SDL_GetWindowSize(m_window, &w, &h);

    m_postprocessing_chain.set_chain(
        chain,
        m_source_buffers,
        w, h
    );
}

void GLRenderer::prepare_gui_rendering() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
}

void GLRenderer::render(RenderData const & render_data, bool gui) {
    GLuint framebuffer = m_postprocessing_chain.empty() ?
        0 : m_source_buffers.framebuffer();
    render_framebuffer(render_data, framebuffer);

    m_postprocessing_chain.render(render_data.camera_data);

    if (gui) {
        render_gui();
    }

    SDL_GL_SwapWindow(m_window);
    glCheckError();
}

void GLRenderer::render_framebuffer(
    RenderData const & render_data,
    GLuint framebuffer
) {
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
    static std::unordered_map<ResourceID, std::vector<MeshRenderData>>
        material_queues;
    for (auto & pair : material_queues) {
        pair.second.resize(0);
    }
    for (MeshRenderData const & mesh_data : render_data.world.mesh_data) {
        material_queues[mesh_data.material_id].push_back(mesh_data);
    }

    std::vector<GLMaterial> const & materials = m_material_manager.materials();
    for (auto const & pair : material_queues) {
        GLMaterial const & material = materials[pair.first];
        GLuint shader_id = material.shader();
        glUseProgram(shader_id);
        glCheckError();

        // Light data
        LightRenderData const & light_data = render_data.world.light_data;

        glUniform3fv(material.view_position_loc(), 1, &render_data.camera_data.view_position[0]);
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
                render_data.camera_data,
                mesh_data
            );
        }
        glCheckError();
    }
}

void GLRenderer::render_gui() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
