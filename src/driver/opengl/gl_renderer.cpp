#include "gl_renderer.h"

#include "src/driver/opengl/gl_shader_utility.h"
#include "src/driver/opengl/gl_utility.h"

#include "glm/gtx/string_cast.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <vector>
#include <unordered_map>
#include <cassert>
#include <iostream>

using namespace prt3;

GLRenderer::GLRenderer(
    SDL_Window * window,
    float downscale_factor
)
 : m_window{window},
   m_downscale_factor{downscale_factor},
   m_material_manager{m_texture_manager},
   m_model_manager{m_material_manager},
   m_selection_shader{nullptr}
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
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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

    PostProcessingPass pixel_pass_info;
    pixel_pass_info.fragment_shader_path =
        "assets/shaders/opengl/pixel_postprocess.fs";
    pixel_pass_info.downscale_factor = downscale_factor;

    PostProcessingPass upscale_pass_info;
    upscale_pass_info.fragment_shader_path =
        "assets/shaders/opengl/passthrough.fs";
    upscale_pass_info.downscale_factor = 1.0f;

    PostProcessingPass editor_pass_info;
    editor_pass_info.fragment_shader_path =
        "assets/shaders/opengl/editor_postprocess.fs";
    editor_pass_info.downscale_factor = 1.0f;

    set_postprocessing_chains(
        PostProcessingChain{{ pixel_pass_info, upscale_pass_info }},
        PostProcessingChain{{ pixel_pass_info, editor_pass_info }}
    );

    m_selection_shader = new GLShader(
        "assets/shaders/opengl/standard.vs",
        "assets/shaders/opengl/write_selected.fs"
    );
}

GLRenderer::~GLRenderer() {
    delete m_selection_shader;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
}

void GLRenderer::set_postprocessing_chains(
    PostProcessingChain const & scene_chain,
    PostProcessingChain const & editor_chain
) {
    int w;
    int h;
    SDL_GetWindowSize(m_window, &w, &h);

    m_scene_postprocessing_chain.set_chain(
        scene_chain,
        m_source_buffers,
        w, h
    );

    m_editor_postprocessing_chain.set_chain(
        editor_chain,
        m_source_buffers,
        w, h
    );
}

NodeID GLRenderer::get_selected(int x, int y) {
    int w;
    int h;
    SDL_GetWindowSize(m_window, &w, &h);

    glFlush();
    glFinish();

    glBindFramebuffer(
        GL_FRAMEBUFFER,
        m_source_buffers.framebuffer()
    );

    glReadBuffer(m_source_buffers.id_attachment());

    NodeID id = NO_NODE;

    GLubyte data[4];
    glReadPixels(
        x / m_downscale_factor,
        (h - y) / m_downscale_factor,
        1,
        1,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        &data
    );

    id = *reinterpret_cast<int32_t*>(data);
    return id;
}

void GLRenderer::prepare_imgui_rendering() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
}

void GLRenderer::render(RenderData const & render_data, bool editor) {
    GLPostProcessingChain & chain = editor ?
        m_editor_postprocessing_chain :
        m_scene_postprocessing_chain;

    GLuint framebuffer = chain.empty() ?
        0 : m_source_buffers.framebuffer();

    render_framebuffer(render_data, framebuffer, false);


    if (!chain.empty()) {
        render_framebuffer(
            render_data,
            m_source_buffers.selection_framebuffer(),
            true
        );
    }

    chain.render(render_data.camera_data, m_frame);
    if (editor) {
        render_imgui();
    }

    SDL_GL_SwapWindow(m_window);
    glCheckError();

    ++m_frame;
}

void GLRenderer::render_framebuffer(
    RenderData const & render_data,
    GLuint framebuffer,
    bool selection_pass
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

    auto const & meshes = m_model_manager.meshes();

    if (!selection_pass) {
        // Render meshes
        // TODO: use a queue based on shaders instead, to reduce state changes
        static std::unordered_map<ResourceID, std::vector<MeshRenderData>>
            material_queues;
        for (auto & pair : material_queues) {
            pair.second.resize(0);
        }
        for (MeshRenderData const & mesh_data : render_data.world.mesh_data) {
            material_queues[mesh_data.material_id].push_back(mesh_data);
        }

        auto const & materials = m_material_manager.materials();

        for (auto const & pair : material_queues) {
            if (pair.second.empty()) { continue; }

            GLMaterial const & material = materials.at(pair.first);
            GLuint shader_id = material.shader().shader();
            glUseProgram(shader_id);
            glCheckError();

            bind_material_data(
                material.shader(),
                material
            );

            // Light data
            LightRenderData const & light_data = render_data.world.light_data;
            bind_light_data(material.shader(), light_data);

            if (!selection_pass) {
                for (MeshRenderData const & mesh_data : pair.second) {
                    bind_mesh_and_camera_data(
                        material.shader(),
                        mesh_data,
                        render_data.camera_data
                    );

                    meshes.at(mesh_data.mesh_id).draw();
                }
            }
            glCheckError();
        }
    } else {
        glUseProgram(m_selection_shader->shader());
        auto const & selected_meshes = render_data.world.selected_mesh_data;
        for (MeshRenderData const & selected_mesh_data : selected_meshes) {
                bind_mesh_and_camera_data(
                    *m_selection_shader,
                    selected_mesh_data,
                    render_data.camera_data
                );

                meshes.at(selected_mesh_data.mesh_id).draw();
            }
    }
}

void GLRenderer::render_imgui() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GLRenderer::bind_light_data(
    GLShader const & s,
    LightRenderData const & light_data
) {
    static const UniformVarString n_point_lights_str = "u_NumberOfPointLights";
    glUniform1i(s.get_uniform_loc(n_point_lights_str), static_cast<int>(light_data.number_of_point_lights));

    static const UniformVarString point_lights_0_pos_str = "u_PointLights[0].position";
    glUniform3fv(s.get_uniform_loc(point_lights_0_pos_str), 1, &light_data.point_lights[0].position[0]);
    static const UniformVarString point_lights_0_col_str = "u_PointLights[0].color";
    glUniform3fv(s.get_uniform_loc(point_lights_0_col_str), 1, &light_data.point_lights[0].light.color[0]);
    static const UniformVarString point_lights_0_a_str = "u_PointLights[0].a";
    glUniform1f(s.get_uniform_loc(point_lights_0_a_str), light_data.point_lights[0].light.quadratic_term);
    static const UniformVarString point_lights_0_b_str = "u_PointLights[0].b";
    glUniform1f(s.get_uniform_loc(point_lights_0_b_str), light_data.point_lights[0].light.linear_term);
    static const UniformVarString point_lights_0_c_str = "u_PointLights[0].c";
    glUniform1f(s.get_uniform_loc(point_lights_0_c_str), light_data.point_lights[0].light.constant_term);

    static const UniformVarString point_lights_1_pos_str = "u_PointLights[1].position";
    glUniform3fv(s.get_uniform_loc(point_lights_1_pos_str), 1, &light_data.point_lights[1].position[0]);
    static const UniformVarString point_lights_1_col_str = "u_PointLights[1].color";
    glUniform3fv(s.get_uniform_loc(point_lights_1_col_str), 1, &light_data.point_lights[1].light.color[0]);
    static const UniformVarString point_lights_1_a_str = "u_PointLights[1].a";
    glUniform1f(s.get_uniform_loc(point_lights_1_a_str), light_data.point_lights[1].light.quadratic_term);
    static const UniformVarString point_lights_1_b_str = "u_PointLights[1].b";
    glUniform1f(s.get_uniform_loc(point_lights_1_b_str), light_data.point_lights[1].light.linear_term);
    static const UniformVarString point_lights_1_c_str = "u_PointLights[1].c";
    glUniform1f(s.get_uniform_loc(point_lights_1_c_str), light_data.point_lights[1].light.constant_term);


    static const UniformVarString point_lights_2_pos_str = "u_PointLights[2].position";
    glUniform3fv(s.get_uniform_loc(point_lights_2_pos_str), 1, &light_data.point_lights[2].position[0]);
    static const UniformVarString point_lights_2_col_str = "u_PointLights[2].color";
    glUniform3fv(s.get_uniform_loc(point_lights_2_col_str), 1, &light_data.point_lights[2].light.color[0]);
    static const UniformVarString point_lights_2_a_str = "u_PointLights[2].a";
    glUniform1f(s.get_uniform_loc(point_lights_2_a_str), light_data.point_lights[2].light.quadratic_term);
    static const UniformVarString point_lights_2_b_str = "u_PointLights[2].b";
    glUniform1f(s.get_uniform_loc(point_lights_2_b_str), light_data.point_lights[2].light.linear_term);
    static const UniformVarString point_lights_2_c_str = "u_PointLights[2].c";
    glUniform1f(s.get_uniform_loc(point_lights_2_c_str), light_data.point_lights[2].light.constant_term);


    static const UniformVarString point_lights_3_pos_str = "u_PointLights[3].position";
    glUniform3fv(s.get_uniform_loc(point_lights_3_pos_str), 1, &light_data.point_lights[3].position[0]);
    static const UniformVarString point_lights_3_col_str = "u_PointLights[3].color";
    glUniform3fv(s.get_uniform_loc(point_lights_3_col_str), 1, &light_data.point_lights[3].light.color[0]);
    static const UniformVarString point_lights_3_a_str = "u_PointLights[3].a";
    glUniform1f(s.get_uniform_loc(point_lights_3_a_str), light_data.point_lights[3].light.quadratic_term);
    static const UniformVarString point_lights_3_b_str = "u_PointLights[3].b";
    glUniform1f(s.get_uniform_loc(point_lights_3_b_str), light_data.point_lights[3].light.linear_term);
    static const UniformVarString point_lights_3_c_str = "u_PointLights[3].c";
    glUniform1f(s.get_uniform_loc(point_lights_3_c_str), light_data.point_lights[3].light.constant_term);

    glm::vec3 dir_light_dir = glm::normalize(light_data.directional_light.direction);
    static const UniformVarString dir_light_dir_str = "u_DirectionalLight.direction";
    glUniform3fv(s.get_uniform_loc(dir_light_dir_str), 1, &dir_light_dir[0]);
    static const UniformVarString dir_light_col_str = "u_DirectionalLight.color";
    glUniform3fv(s.get_uniform_loc(dir_light_col_str), 1, &light_data.directional_light.color[0]);

    static const UniformVarString dir_light_on_str = "u_DirectionalLightOn";
    glUniform1i(s.get_uniform_loc(dir_light_on_str), static_cast<int>(light_data.directional_light_on));

    static const UniformVarString ambient_light_str = "u_AmbientLight";
    glUniform3fv(s.get_uniform_loc(ambient_light_str), 1, &light_data.ambient_light.color[0]);
}

void GLRenderer::bind_mesh_and_camera_data(
    GLShader const & s,
    MeshRenderData const & mesh_data,
    CameraRenderData const & camera_data
) {
    glm::mat4 m_matrix = mesh_data.transform;
    glm::mat4 mv_matrix = camera_data.view_matrix * mesh_data.transform;
    glm::mat4 mvp_matrix = camera_data.projection_matrix * mv_matrix;
    glm::mat3 inv_tpos_matrix = glm::inverse(glm::transpose(m_matrix));

    static const UniformVarString view_pos_str = "u_ViewPosition";
    glUniform3fv(s.get_uniform_loc(view_pos_str), 1, &camera_data.view_position[0]);

    static const UniformVarString mmatrix_str = "u_MMatrix";
    glUniformMatrix4fv(s.get_uniform_loc(mmatrix_str), 1, GL_FALSE, &m_matrix[0][0]);
    static const UniformVarString mvmatrix_str = "u_MVMatrix";
    glUniformMatrix4fv(s.get_uniform_loc(mvmatrix_str), 1, GL_FALSE, &mv_matrix[0][0]);
    static const UniformVarString mvpmatrix_str = "u_MVPMatrix";
    glUniformMatrix4fv(s.get_uniform_loc(mvpmatrix_str), 1, GL_FALSE, &mvp_matrix[0][0]);
    static const UniformVarString inv_tpos_matrix_str = "u_InvTposMMatrix";
    glUniformMatrix3fv(s.get_uniform_loc(inv_tpos_matrix_str), 1, GL_FALSE, &inv_tpos_matrix[0][0]);
    static const UniformVarString node_str = "u_ID";
    GLuint u_node_id = static_cast<GLuint>(mesh_data.node);
    glUniform1ui(s.get_uniform_loc(node_str), GLuint(u_node_id));
}

void GLRenderer::bind_material_data(
    GLShader const & s,
    GLMaterial const & material
) {
    static const UniformVarString albedo_map_str = "u_AlbedoMap";
    glUniform1i(s.get_uniform_loc(albedo_map_str), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material.albedo_map());

    static const UniformVarString normal_map_str = "u_NormalMap";
    glUniform1i(s.get_uniform_loc(normal_map_str), 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material.normal_map());

    static const UniformVarString metallic_map_str = "u_MetallicMap";
    glUniform1i(s.get_uniform_loc(metallic_map_str), 2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, material.metallic_map());

    static const UniformVarString roughness_map_str = "u_RoughnessMap";
    glUniform1i(s.get_uniform_loc(roughness_map_str), 3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, material.roughness_map());

    static const UniformVarString albedo_str = "u_Albedo";
    glUniform4fv(s.get_uniform_loc(albedo_str), 1, &material.material().albedo[0]);
    static const UniformVarString metallic_str = "u_Metallic";
    glUniform1f(s.get_uniform_loc(metallic_str), material.material().metallic);
    static const UniformVarString roughness_str = "u_Roughness";
    glUniform1f(s.get_uniform_loc(roughness_str), material.material().roughness);
}
