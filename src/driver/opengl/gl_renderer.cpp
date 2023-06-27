#include "gl_renderer.h"

#include "src/driver/opengl/gl_shader_utility.h"
#include "src/driver/opengl/gl_utility.h"
#include "src/util/log.h"

#include "glm/gtx/string_cast.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <vector>
#include <unordered_map>
#include <cassert>

using namespace prt3;

GLRenderer::GLRenderer(
    GLFWwindow * window,
    float downscale_factor
)
 : m_window{window},
   m_downscale_factor{downscale_factor},
   m_material_manager{m_texture_manager},
   m_model_manager{m_material_manager},
   m_selection_shader{nullptr},
   m_animated_selection_shader{nullptr}
  {
    EmscriptenWebGLContextAttributes attrs{};
	attrs.antialias = true;
	attrs.majorVersion = 2;
	attrs.minorVersion = 0;
	attrs.alpha = false;
    attrs.enableExtensionsByDefault = true;
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context =
        emscripten_webgl_create_context("#canvas", &attrs);

	if (emscripten_webgl_make_context_current(webgl_context) !=
        EMSCRIPTEN_RESULT_SUCCESS) {
        PRT3LOG("%s\n", "Failed to make context current.");
    }

    /* Enable GL functionality */
    glEnable(GL_DEPTH_TEST);
    glCheckError();
    glDepthFunc(GL_LESS);
    glCheckError();
    glEnable(GL_CULL_FACE);
    glCheckError();
    glCullFace(GL_BACK);
    glCheckError();

    m_texture_manager.init();
    m_material_manager.init();

    ImGui_ImplGlfw_InitForOpenGL(m_window, false);
    glCheckError();
    ImGui_ImplOpenGL3_Init("#version 300 es");
    glCheckError();

    int w;
    int h;
    glfwGetWindowSize(m_window, &w, &h);
    glCheckError();
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

    m_animated_selection_shader = new GLShader(
        "assets/shaders/opengl/standard_animated.vs",
        "assets/shaders/opengl/write_selected.fs"
    );

    m_transparency_blend_shader = new GLShader(
        "assets/shaders/opengl/passthrough.vs",
        "assets/shaders/opengl/transparency_blend_shader.fs"
    );
}

GLRenderer::~GLRenderer() {
    delete m_selection_shader;
    delete m_animated_selection_shader;
    delete m_transparency_blend_shader;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void GLRenderer::set_postprocessing_chains(
    PostProcessingChain const & scene_chain,
    PostProcessingChain const & editor_chain
) {
    int w;
    int h;
    glfwGetWindowSize(m_window, &w, &h);

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
    glfwGetWindowSize(m_window, &w, &h);

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
    ImGui_ImplGlfw_NewFrame();
}

void GLRenderer::render(RenderData const & render_data, bool editor) {
    GLPostProcessingChain & chain = editor ?
        m_editor_postprocessing_chain :
        m_scene_postprocessing_chain;

    render_framebuffer(render_data, editor, PassType::opaque);
    render_framebuffer(render_data, editor, PassType::transparent);

    if (!chain.empty()) {
        render_framebuffer(
            render_data,
            editor,
            PassType::selection
        );
    }

    chain.render(render_data.camera_data, m_frame);
    if (editor) {
        render_imgui();
    }

    glfwSwapBuffers(m_window);
    glCheckError();

    ++m_frame;
}

void GLRenderer::render_framebuffer(
    RenderData const & render_data,
    bool editor,
    PassType type
) {
    GLPostProcessingChain const & chain = editor ?
        m_editor_postprocessing_chain :
        m_scene_postprocessing_chain;

    GLuint opaque_framebuffer = chain.empty() ?
        0 : m_source_buffers.framebuffer();

    GLuint framebuffer;
    bool transparent = false;
    switch (type) {
        case PassType::opaque:
            framebuffer = opaque_framebuffer;
            break;
        case PassType::transparent:
            transparent = true;
            framebuffer = m_source_buffers.accum_framebuffer();
            break;
        case PassType::selection:
            framebuffer = m_source_buffers.selection_framebuffer();
            break;
    }

    // Bind the framebuffer
    int w;
    int h;
    glfwGetWindowSize(m_window, &w, &h);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glCheckError();
    glViewport(0, 0,
               static_cast<GLint>(w / m_downscale_factor),
               static_cast<GLint>(h / m_downscale_factor));
    glCheckError();

    GLenum attachments[3] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
    };

    if (transparent) {
        glDepthMask(GL_FALSE);
        glCheckError();
        glEnable(GL_BLEND);
        glCheckError();
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
        glCheckError();
        glBlendEquation(GL_FUNC_ADD);
        glCheckError();

        glDrawBuffers(2, attachments);
        glCheckError();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glCheckError();
        glClear(GL_COLOR_BUFFER_BIT);
        glCheckError();
    } else {
        glDepthMask(GL_TRUE);
        glCheckError();
        glDisable(GL_BLEND);
        glCheckError();

        glDrawBuffers(3, attachments);
        glCheckError();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glCheckError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCheckError();
    }

    auto const & meshes = m_model_manager.meshes();

    if (type != PassType::selection) {
        auto const & materials = m_material_manager.materials();

        static std::unordered_map<GLShader const *, std::vector<MeshRenderData> >
            shader_queues;

        static std::unordered_map<GLShader const *, std::vector<AnimatedMeshRenderData> >
            animated_shader_queues;

        for (auto & pair : shader_queues) {
            pair.second.resize(0);
        }

        for (auto & pair : animated_shader_queues) {
            pair.second.resize(0);
        }

        for (MeshRenderData const & mesh_data : render_data.world.mesh_data) {
            GLMaterial const & mat = materials.at(mesh_data.material_id);
            if (mat.material().transparent != transparent) {
                continue;
            }
            GLShader const * shader =
                &materials.at(mesh_data.material_id).get_shader(
                    false,
                    transparent
                );
            shader_queues[shader].push_back(mesh_data);
        }

        for (AnimatedMeshRenderData const & data : render_data.world.animated_mesh_data) {
            GLMaterial const & mat = materials.at(data.mesh_data.material_id);
            if (mat.material().transparent != transparent) {
                continue;
            }
            GLShader const * shader =
                &materials.at(data.mesh_data.material_id).get_shader(
                    true,
                    transparent
                );
            animated_shader_queues[shader].push_back(data);
        }

        for (auto const & pair : shader_queues) {
            if (pair.second.empty()) { continue; }

            GLShader const & shader = *pair.first;
            GLuint shader_id = shader.shader();
            glUseProgram(shader_id);
            glCheckError();

            // Light data
            LightRenderData const & light_data = render_data.world.light_data;
            bind_light_data(shader, light_data);

            for (MeshRenderData const & mesh_data : pair.second) {
                GLMaterial const & material = materials.at(mesh_data.material_id);

                bind_material_data(
                    shader,
                    material
                );

                bind_transform_and_camera_data(
                    shader,
                    mesh_data.transform,
                    render_data.camera_data
                );
                bind_node_data(
                    shader,
                    mesh_data.node
                );
                meshes.at(mesh_data.mesh_id).draw_elements_triangles();
            }
            glCheckError();
        }

        for (auto const & pair : animated_shader_queues) {
            if (pair.second.empty()) { continue; }

            GLShader const & shader = *pair.first;
            GLuint shader_id = shader.shader();
            glUseProgram(shader_id);
            glCheckError();

            // Light data
            LightRenderData const & light_data = render_data.world.light_data;
            bind_light_data(shader, light_data);

            for (AnimatedMeshRenderData const & data : pair.second) {
                MeshRenderData const & mesh_data = data.mesh_data;
                GLMaterial const & material = materials.at(mesh_data.material_id);

                bind_material_data(
                    shader,
                    material
                );

                bind_transform_and_camera_data(
                    shader,
                    mesh_data.transform,
                    render_data.camera_data
                );
                bind_node_data(
                    shader,
                    mesh_data.node
                );
                bind_bone_data(
                    shader,
                    render_data.world.bone_data[data.bone_data_index]
                );

                meshes.at(mesh_data.mesh_id).draw_elements_triangles();
            }
            glCheckError();
        }

        /* Wireframes */
        if (!transparent) {
            glDrawBuffers(1, attachments);
            glCheckError();

            glEnable(GL_POLYGON_OFFSET_FILL);
            glCheckError();
            glPolygonOffset(1.0, 1.0);
            glCheckError();

            GLShader const & shader = m_material_manager.wireframe_shader();
            glUseProgram(shader.shader());
            glCheckError();

            EditorRenderData const & editor_data = render_data.editor_data;
            for (WireframeRenderData const & data : editor_data.line_data) {
                bind_transform_and_camera_data(
                    shader,
                    data.transform,
                    render_data.camera_data
                );

                static const GLVarString color_str = "u_Color";
                glUniform4fv(shader.get_uniform_loc(color_str), 1, &data.color[0]);
                glCheckError();

                meshes.at(data.mesh_id).draw_array_lines();
            }

            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        if (transparent) {
            GLShader & shader = *m_transparency_blend_shader;
            glBindFramebuffer(GL_FRAMEBUFFER, opaque_framebuffer);
            glCheckError();
            glDrawBuffers(1, attachments);
            glCheckError();
            glUseProgram(shader.shader());
            glCheckError();
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glCheckError();

            static const GLVarString accum_str = "uAccumulate";
            glUniform1i(shader.get_uniform_loc(accum_str), 0);
            glCheckError();
            glActiveTexture(GL_TEXTURE0);
            glCheckError();
            glBindTexture(GL_TEXTURE_2D, m_source_buffers.accum_texture());
            glCheckError();

            static const GLVarString accum_alpha_str = "uAccumulateAlpha";
            glUniform1i(shader.get_uniform_loc(accum_alpha_str), 1);
            glCheckError();
            glActiveTexture(GL_TEXTURE1);
            glCheckError();
            glBindTexture(GL_TEXTURE_2D, m_source_buffers.accum_alpha_texture());
            glCheckError();

            glBindVertexArray(chain.screen_quad_vao());
            glCheckError();
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glCheckError();
            glBindVertexArray(0);
            glCheckError();
        }
    } else {
        glUseProgram(m_selection_shader->shader());
        glCheckError();
        auto const & selected_meshes = render_data.world.selected_mesh_data;
        for (MeshRenderData const & selected_mesh_data : selected_meshes) {
            bind_transform_and_camera_data(
                *m_selection_shader,
                selected_mesh_data.transform,
                render_data.camera_data
            );
            bind_node_data(
                *m_selection_shader,
                selected_mesh_data.node
            );

            meshes.at(selected_mesh_data.mesh_id).draw_elements_triangles();
        }

        glUseProgram(m_animated_selection_shader->shader());
        glCheckError();
        for (AnimatedMeshRenderData const & data :
            render_data.world.selected_animated_mesh_data) {
            MeshRenderData const & mesh_data = data.mesh_data;

            bind_transform_and_camera_data(
                *m_animated_selection_shader,
                mesh_data.transform,
                render_data.camera_data
            );
            bind_node_data(
                *m_animated_selection_shader,
                mesh_data.node
            );
            bind_bone_data(
                *m_animated_selection_shader,
                render_data.world.bone_data[data.bone_data_index]
            );

            meshes.at(mesh_data.mesh_id).draw_elements_triangles();
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
    static const GLVarString n_point_lights_str = "u_NumberOfPointLights";
    glUniform1i(s.get_uniform_loc(n_point_lights_str), static_cast<int>(light_data.number_of_point_lights));

    static const GLVarString point_lights_0_pos_str = "u_PointLights[0].position";
    glUniform3fv(s.get_uniform_loc(point_lights_0_pos_str), 1, &light_data.point_lights[0].position[0]);
    static const GLVarString point_lights_0_col_str = "u_PointLights[0].color";
    glUniform3fv(s.get_uniform_loc(point_lights_0_col_str), 1, &light_data.point_lights[0].light.color[0]);
    static const GLVarString point_lights_0_a_str = "u_PointLights[0].a";
    glUniform1f(s.get_uniform_loc(point_lights_0_a_str), light_data.point_lights[0].light.quadratic_term);
    static const GLVarString point_lights_0_b_str = "u_PointLights[0].b";
    glUniform1f(s.get_uniform_loc(point_lights_0_b_str), light_data.point_lights[0].light.linear_term);
    static const GLVarString point_lights_0_c_str = "u_PointLights[0].c";
    glUniform1f(s.get_uniform_loc(point_lights_0_c_str), light_data.point_lights[0].light.constant_term);

    static const GLVarString point_lights_1_pos_str = "u_PointLights[1].position";
    glUniform3fv(s.get_uniform_loc(point_lights_1_pos_str), 1, &light_data.point_lights[1].position[0]);
    static const GLVarString point_lights_1_col_str = "u_PointLights[1].color";
    glUniform3fv(s.get_uniform_loc(point_lights_1_col_str), 1, &light_data.point_lights[1].light.color[0]);
    static const GLVarString point_lights_1_a_str = "u_PointLights[1].a";
    glUniform1f(s.get_uniform_loc(point_lights_1_a_str), light_data.point_lights[1].light.quadratic_term);
    static const GLVarString point_lights_1_b_str = "u_PointLights[1].b";
    glUniform1f(s.get_uniform_loc(point_lights_1_b_str), light_data.point_lights[1].light.linear_term);
    static const GLVarString point_lights_1_c_str = "u_PointLights[1].c";
    glUniform1f(s.get_uniform_loc(point_lights_1_c_str), light_data.point_lights[1].light.constant_term);


    static const GLVarString point_lights_2_pos_str = "u_PointLights[2].position";
    glUniform3fv(s.get_uniform_loc(point_lights_2_pos_str), 1, &light_data.point_lights[2].position[0]);
    static const GLVarString point_lights_2_col_str = "u_PointLights[2].color";
    glUniform3fv(s.get_uniform_loc(point_lights_2_col_str), 1, &light_data.point_lights[2].light.color[0]);
    static const GLVarString point_lights_2_a_str = "u_PointLights[2].a";
    glUniform1f(s.get_uniform_loc(point_lights_2_a_str), light_data.point_lights[2].light.quadratic_term);
    static const GLVarString point_lights_2_b_str = "u_PointLights[2].b";
    glUniform1f(s.get_uniform_loc(point_lights_2_b_str), light_data.point_lights[2].light.linear_term);
    static const GLVarString point_lights_2_c_str = "u_PointLights[2].c";
    glUniform1f(s.get_uniform_loc(point_lights_2_c_str), light_data.point_lights[2].light.constant_term);


    static const GLVarString point_lights_3_pos_str = "u_PointLights[3].position";
    glUniform3fv(s.get_uniform_loc(point_lights_3_pos_str), 1, &light_data.point_lights[3].position[0]);
    static const GLVarString point_lights_3_col_str = "u_PointLights[3].color";
    glUniform3fv(s.get_uniform_loc(point_lights_3_col_str), 1, &light_data.point_lights[3].light.color[0]);
    static const GLVarString point_lights_3_a_str = "u_PointLights[3].a";
    glUniform1f(s.get_uniform_loc(point_lights_3_a_str), light_data.point_lights[3].light.quadratic_term);
    static const GLVarString point_lights_3_b_str = "u_PointLights[3].b";
    glUniform1f(s.get_uniform_loc(point_lights_3_b_str), light_data.point_lights[3].light.linear_term);
    static const GLVarString point_lights_3_c_str = "u_PointLights[3].c";
    glUniform1f(s.get_uniform_loc(point_lights_3_c_str), light_data.point_lights[3].light.constant_term);

    glm::vec3 dir_light_dir = glm::normalize(light_data.directional_light.direction);
    static const GLVarString dir_light_dir_str = "u_DirectionalLight.direction";
    glUniform3fv(s.get_uniform_loc(dir_light_dir_str), 1, &dir_light_dir[0]);
    static const GLVarString dir_light_col_str = "u_DirectionalLight.color";
    glUniform3fv(s.get_uniform_loc(dir_light_col_str), 1, &light_data.directional_light.color[0]);

    static const GLVarString dir_light_on_str = "u_DirectionalLightOn";
    glUniform1i(s.get_uniform_loc(dir_light_on_str), static_cast<int>(light_data.directional_light_on));

    static const GLVarString ambient_light_str = "u_AmbientLight";
    glUniform3fv(s.get_uniform_loc(ambient_light_str), 1, &light_data.ambient_light.color[0]);
}

void GLRenderer::bind_transform_and_camera_data(
    GLShader const & s,
    glm::mat4 const & transform,
    CameraRenderData const & camera_data
) {
    glm::mat4 m_matrix = transform;
    glm::mat4 mv_matrix = camera_data.view_matrix * transform;
    glm::mat4 mvp_matrix = camera_data.projection_matrix * mv_matrix;
    glm::mat3 inv_tpos_matrix = glm::inverse(glm::transpose(m_matrix));

    static const GLVarString view_pos_str = "u_ViewPosition";
    glUniform3fv(s.get_uniform_loc(view_pos_str), 1, &camera_data.view_position[0]);

    static const GLVarString mmatrix_str = "u_MMatrix";
    glUniformMatrix4fv(s.get_uniform_loc(mmatrix_str), 1, GL_FALSE, &m_matrix[0][0]);
    static const GLVarString mvmatrix_str = "u_MVMatrix";
    glUniformMatrix4fv(s.get_uniform_loc(mvmatrix_str), 1, GL_FALSE, &mv_matrix[0][0]);
    static const GLVarString mvpmatrix_str = "u_MVPMatrix";
    glUniformMatrix4fv(s.get_uniform_loc(mvpmatrix_str), 1, GL_FALSE, &mvp_matrix[0][0]);
    static const GLVarString inv_tpos_matrix_str = "u_InvTposMMatrix";
    glUniformMatrix3fv(s.get_uniform_loc(inv_tpos_matrix_str), 1, GL_FALSE, &inv_tpos_matrix[0][0]);
}

void GLRenderer::bind_node_data(
    GLShader const & shader,
    NodeID node_id
) {
    static const GLVarString node_str = "u_ID";
    GLuint u_node_id = static_cast<GLuint>(node_id);
    glUniform1ui(shader.get_uniform_loc(node_str), GLuint(u_node_id));
    glCheckError();
}

void GLRenderer::bind_material_data(
    GLShader const & s,
    GLMaterial const & material
) {
    static const GLVarString albedo_map_str = "u_AlbedoMap";
    glUniform1i(s.get_uniform_loc(albedo_map_str), 0);
    glActiveTexture(GL_TEXTURE0);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.albedo_map());
    glCheckError();

    static const GLVarString normal_map_str = "u_NormalMap";
    glUniform1i(s.get_uniform_loc(normal_map_str), 1);
    glCheckError();
    glActiveTexture(GL_TEXTURE1);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.normal_map());
    glCheckError();

    static const GLVarString metallic_map_str = "u_MetallicMap";
    glUniform1i(s.get_uniform_loc(metallic_map_str), 2);
    glCheckError();
    glActiveTexture(GL_TEXTURE2);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.metallic_map());
    glCheckError();

    static const GLVarString roughness_map_str = "u_RoughnessMap";
    glUniform1i(s.get_uniform_loc(roughness_map_str), 3);
    glCheckError();
    glActiveTexture(GL_TEXTURE3);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.roughness_map());
    glCheckError();

    static const GLVarString albedo_str = "u_Albedo";
    glUniform4fv(s.get_uniform_loc(albedo_str), 1, &material.material().albedo[0]);
    glCheckError();
    static const GLVarString metallic_str = "u_Metallic";
    glUniform1f(s.get_uniform_loc(metallic_str), material.material().metallic);
    glCheckError();
    static const GLVarString roughness_str = "u_Roughness";
    glUniform1f(s.get_uniform_loc(roughness_str), material.material().roughness);
    glCheckError();
}

void GLRenderer::bind_bone_data(
    GLShader const & s,
    BoneData const & bone_data
) {
    static const GLVarString bones_str = "u_Bones";
    glUniformMatrix4fv(s.get_uniform_loc(bones_str), bone_data.bones.size(), GL_FALSE, &bone_data.bones[0][0][0]);
    glCheckError();
}
