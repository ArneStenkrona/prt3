#include "gl_renderer.h"

#include "src/backend/opengl/gl_shader_utility.h"
#include "src/backend/opengl/gl_utility.h"
#include "src/util/log.h"
#include "src/util/mesh_util.h"

#include "glm/gtx/string_cast.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_opengl3.h"

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <vector>
#include <unordered_map>
#include <cassert>
#include <cstring>

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
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glDepthFunc(GL_LESS));
    GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glCullFace(GL_BACK));

    m_texture_manager.init();
    m_material_manager.init();

    ImGui_ImplGlfw_InitForOpenGL(m_window, false);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    int w;
    int h;
    glfwGetWindowSize(m_window, &w, &h);
    GLint buffer_width = static_cast<GLint>(w / m_downscale_factor);
    GLint buffer_height = static_cast<GLint>(h / m_downscale_factor);
    m_source_buffers.init(buffer_width, buffer_height);

    PostProcessingPass pixel_pass_info;
    pixel_pass_info.fragment_shader_path =
        "assets/shaders/opengl/pixel_postprocess.fs";
    pixel_pass_info.downscale_factor = downscale_factor;

    PostProcessingPass game_pass_info;
    game_pass_info.fragment_shader_path =
        "assets/shaders/opengl/game_postprocess.fs";
    game_pass_info.downscale_factor = 1.0f;

    PostProcessingPass editor_pass_info;
    editor_pass_info.fragment_shader_path =
        "assets/shaders/opengl/editor_postprocess.fs";
    editor_pass_info.downscale_factor = 1.0f;

    set_postprocessing_chains(
        PostProcessingChain{{ game_pass_info, pixel_pass_info  }},
        PostProcessingChain{{ pixel_pass_info, editor_pass_info }}
    );

    m_decal_shader = new GLShader(
        glshaderutility::create_shader(
            "assets/shaders/opengl/decal.vs",
            "assets/shaders/opengl/decal.fs"
        )
    );

    m_selection_shader = new GLShader(
        glshaderutility::create_shader(
            "assets/shaders/opengl/standard.vs",
            "assets/shaders/opengl/write_selected.fs"
        )
    );

    m_animated_selection_shader = new GLShader(
        glshaderutility::create_shader(
            "assets/shaders/opengl/standard_animated.vs",
            "assets/shaders/opengl/write_selected.fs"
        )
    );

    m_transparency_blend_shader = new GLShader(
        glshaderutility::create_shader(
            "assets/shaders/opengl/passthrough.vs",
            "assets/shaders/opengl/transparency_blend_shader.fs"
        )
    );

    m_canvas_shader = new GLShader(
         glshaderutility::create_shader(
            "assets/shaders/opengl/canvas.vs",
            "assets/shaders/opengl/canvas.fs"
         )
    );

    /* init decal objects */
    std::array<glm::vec3, 36> decal_vertices;
    insert_box(glm::vec3{-0.5f}, glm::vec3{0.5f}, decal_vertices.data());

    m_decal_mesh = m_model_manager.upload_pos_mesh(
        decal_vertices.data(),
        decal_vertices.size()
    );

    /* init canvas objects */
    GL_CHECK(glGenVertexArrays(1, &m_canvas_vao));
    GL_CHECK(glBindVertexArray(m_canvas_vao));
    GL_CHECK(glGenBuffers(1, &m_canvas_vbo));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_canvas_vbo));

    static const GLVarString pos_uv_str = "a_PosUV";
    GLint pos_uv_attr;
    GL_CHECK(pos_uv_attr = m_canvas_shader->get_attrib_loc(pos_uv_str));
    GL_CHECK(glEnableVertexAttribArray(pos_uv_attr));
    GL_CHECK(glVertexAttribPointer(
        pos_uv_attr,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(CanvasGeometry),
        0
    ));

    static const GLVarString color_str = "a_Color";
    GLint color_attr;
    GL_CHECK(color_attr = m_canvas_shader->get_attrib_loc(color_str));
    GL_CHECK(glEnableVertexAttribArray(color_attr));
    GL_CHECK(glVertexAttribPointer(
        color_attr,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(CanvasGeometry),
        reinterpret_cast<void*>(offsetof(CanvasGeometry, color))
    ));

    GL_CHECK(glBindVertexArray(0));
}

GLRenderer::~GLRenderer() {
    delete m_selection_shader;
    delete m_animated_selection_shader;
    delete m_transparency_blend_shader;
    m_model_manager.free_pos_mesh(m_decal_mesh);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    /* free canvas objects */
    GL_CHECK(glDeleteBuffers(1, &m_canvas_vbo));
    GL_CHECK(glDeleteBuffers(1, &m_canvas_vao));

    GL_CHECK(glDeleteProgram(m_decal_shader->shader()));
    GL_CHECK(glDeleteProgram(m_selection_shader->shader()));
    GL_CHECK(glDeleteProgram(m_animated_selection_shader->shader()));
    GL_CHECK(glDeleteProgram(m_transparency_blend_shader->shader()));
    GL_CHECK(glDeleteProgram(m_canvas_shader->shader()));
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

    GL_CHECK(glFlush());
    GL_CHECK(glFinish());

    GL_CHECK(glBindFramebuffer(
        GL_FRAMEBUFFER,
        m_source_buffers.framebuffer()
    ));

    GL_CHECK(glReadBuffer(m_source_buffers.node_data_attachment()));

    GLubyte data[4];
    GL_CHECK(glReadPixels(
        x / m_downscale_factor,
        (h - y) / m_downscale_factor,
        1,
        1,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        &data
    ));

    uint32_t raw_id = 0;
    memcpy(&raw_id, data, 3);
    raw_id = raw_id & 0x00ffffffu;

    NodeID id;
    if (raw_id == 0x00ffffffu) {
        id = NO_NODE;
    } else {
        memcpy(&id, &raw_id, sizeof(NodeID));
    }

    return id;
}

void GLRenderer::prepare_imgui_rendering() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

static bool is_transparent(
    MeshRenderData const & mesh_data,
    GLMaterial const & mat
) {
    MaterialOverride const & mo = mesh_data.material_override;
    return mat.material().transparent || (mo.tint_active && mo.tint.a < 1.0f);
}

void GLRenderer::render(RenderData & render_data, bool editor) {
    GLPostProcessingChain & chain = editor ?
        m_editor_postprocessing_chain :
        m_scene_postprocessing_chain;

    render_framebuffer(render_data, editor, PassType::opaque);
    render_framebuffer(render_data, editor, PassType::transparent);
    render_framebuffer(render_data, editor, PassType::decal);

    if (!chain.empty()) {
        render_framebuffer(
            render_data,
            editor,
            PassType::selection
        );
    }

    chain.render(render_data.camera_data, m_frame);

    render_canvas(render_data.scene.canvas_data);

    if (editor) {
        render_imgui();
    }

    glfwSwapBuffers(m_window);

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
    bool decal = false;
    switch (type) {
        case PassType::decal:
            framebuffer = m_source_buffers.decal_framebuffer();
            decal = true;
            break;
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
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));

    GLint view_w = static_cast<GLint>(w / m_downscale_factor);
    GLint view_h = static_cast<GLint>(h / m_downscale_factor);

    GL_CHECK(glViewport(0, 0, view_w, view_h));

    GLenum attachments[3] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
    };

    if (decal) {
        GL_CHECK(glDepthMask(GL_FALSE));
        GL_CHECK(glDisable(GL_DEPTH_TEST));
        GL_CHECK(glEnable(GL_CULL_FACE));
        GL_CHECK(glCullFace(GL_FRONT));
        GL_CHECK(glDisable(GL_BLEND));
        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        GL_CHECK(glBlendEquation(GL_FUNC_ADD));

        GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

        GL_CHECK(glDrawBuffers(1, attachments));
    } else if (transparent) {
        GL_CHECK(glDepthMask(GL_FALSE));
        GL_CHECK(glEnable(GL_DEPTH_TEST));
        GL_CHECK(glEnable(GL_CULL_FACE));
        GL_CHECK(glCullFace(GL_BACK));

        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA));
        GL_CHECK(glBlendEquation(GL_FUNC_ADD));

        GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

        GL_CHECK(glDrawBuffers(2, attachments));

        GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
    } else {
        GL_CHECK(glDepthMask(GL_TRUE));
        GL_CHECK(glEnable(GL_DEPTH_TEST));
        GL_CHECK(glEnable(GL_CULL_FACE));
        GL_CHECK(glCullFace(GL_BACK));

        GL_CHECK(glDisable(GL_BLEND));

        GL_CHECK(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

        GL_CHECK(glDrawBuffers(3, attachments));

        GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    auto const & meshes = m_model_manager.meshes();
    if (type == PassType::decal) {
        GL_CHECK(glUseProgram(m_decal_shader->shader()));
        bind_decal_data(*m_decal_shader, render_data.camera_data);

        auto const & decal_data = render_data.scene.decal_data;
        for (DecalRenderData const & data : decal_data) {
            static const GLVarString decal_map_str = "u_DecalMap";
            bind_texture(
                *m_decal_shader,
                decal_map_str,
                2,
                m_texture_manager.get_texture(data.texture)
            );

            bind_transform_and_camera_data(
                *m_decal_shader,
                data.transform,
                render_data.camera_data
            );

            static const GLVarString inv_mmatrix_str = "u_InvMMatrix";
            glm::mat4 inv_mmatrix = glm::inverse(data.transform);
            GL_CHECK(glUniformMatrix4fv(
                m_decal_shader->get_uniform_loc(inv_mmatrix_str),
                1,
                GL_FALSE,
                &inv_mmatrix[0][0]
            ));

            m_model_manager.meshes().at(m_decal_mesh).draw_array_triangles();
        }
    } else if (type == PassType::opaque || type == PassType::transparent) {
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

        for (MeshRenderData const & mesh_data : render_data.scene.mesh_data) {
            GLMaterial const & mat = materials.at(mesh_data.material_id);
            if (is_transparent(mesh_data, mat) != transparent) {
                continue;
            }
            GLShader const * shader =
                &materials.at(mesh_data.material_id).get_shader(
                    false,
                    transparent
                );
            shader_queues[shader].push_back(mesh_data);
        }

        for (AnimatedMeshRenderData const & data : render_data.scene.animated_mesh_data) {
            GLMaterial const & mat = materials.at(data.mesh_data.material_id);
            if (is_transparent(data.mesh_data, mat) != transparent) {
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
            GL_CHECK(glUseProgram(shader_id));

            // Light data
            LightRenderData const & light_data = render_data.scene.light_data;
            bind_light_data(shader, light_data);

            for (MeshRenderData const & mesh_data : pair.second) {
                GLMaterial const & material = materials.at(mesh_data.material_id);

                bind_material_data(
                    shader,
                    material,
                    mesh_data.material_override
                );

                bind_transform_and_camera_data(
                    shader,
                    mesh_data.transform,
                    render_data.camera_data
                );

                bind_node_data(
                    shader,
                    mesh_data.node_data
                );

                meshes.at(mesh_data.mesh_id).draw_elements_triangles();
            }
        }

        for (auto const & pair : animated_shader_queues) {
            if (pair.second.empty()) { continue; }

            GLShader const & shader = *pair.first;
            GLuint shader_id = shader.shader();
            GL_CHECK(glUseProgram(shader_id));

            // Light data
            LightRenderData const & light_data = render_data.scene.light_data;
            bind_light_data(shader, light_data);

            for (AnimatedMeshRenderData const & data : pair.second) {
                MeshRenderData const & mesh_data = data.mesh_data;
                GLMaterial const & material = materials.at(mesh_data.material_id);

                bind_material_data(
                    shader,
                    material,
                    mesh_data.material_override
                );

                bind_transform_and_camera_data(
                    shader,
                    mesh_data.transform,
                    render_data.camera_data
                );

                bind_node_data(
                    shader,
                    mesh_data.node_data
                );

                bind_bone_data(
                    shader,
                    render_data.scene.bone_data[data.bone_data_index]
                );

                meshes.at(mesh_data.mesh_id).draw_elements_triangles();
            }
        }

        /* Wireframes */
        if (!transparent) {
            GL_CHECK(glDrawBuffers(1, attachments));

            GL_CHECK(glEnable(GL_POLYGON_OFFSET_FILL));
            GL_CHECK(glPolygonOffset(1.0, 1.0));

            GLShader const & shader = m_material_manager.wireframe_shader();
            GL_CHECK(glUseProgram(shader.shader()));

            EditorRenderData const & editor_data = render_data.editor_data;
            for (WireframeRenderData const & data : editor_data.line_data) {
                bind_transform_and_camera_data(
                    shader,
                    data.transform,
                    render_data.camera_data
                );

                static const GLVarString color_str = "u_Color";
                GL_CHECK(glUniform4fv(shader.get_uniform_loc(color_str), 1, &data.color[0]));

                meshes.at(data.mesh_id).draw_array_lines();
            }

            GL_CHECK(glDisable(GL_POLYGON_OFFSET_FILL));
        }

        if (transparent) {
            GLShader & shader = *m_transparency_blend_shader;
            GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, opaque_framebuffer));
            GL_CHECK(glDrawBuffers(1, attachments));
            GL_CHECK(glUseProgram(shader.shader()));
            GL_CHECK(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));

            static const GLVarString accum_str = "uAccumulate";
            GL_CHECK(glUniform1i(shader.get_uniform_loc(accum_str), 0));
            GL_CHECK(glActiveTexture(GL_TEXTURE0));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_source_buffers.accum_texture()));

            static const GLVarString accum_alpha_str = "uAccumulateAlpha";
            GL_CHECK(glUniform1i(shader.get_uniform_loc(accum_alpha_str), 1));
            GL_CHECK(glActiveTexture(GL_TEXTURE1));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_source_buffers.accum_alpha_texture()));

            GL_CHECK(glBindVertexArray(chain.screen_quad_vao()));
            GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
            GL_CHECK(glBindVertexArray(0));
        }
    } else {
        GL_CHECK(glUseProgram(m_selection_shader->shader()));
        auto const & selected_meshes = render_data.scene.selected_mesh_data;
        for (MeshRenderData const & selected_mesh_data : selected_meshes) {
            bind_transform_and_camera_data(
                *m_selection_shader,
                selected_mesh_data.transform,
                render_data.camera_data
            );
            bind_node_data(
                *m_selection_shader,
                selected_mesh_data.node_data
            );

            meshes.at(selected_mesh_data.mesh_id).draw_elements_triangles();
        }

        GL_CHECK(glUseProgram(m_animated_selection_shader->shader()));
        for (AnimatedMeshRenderData const & data :
            render_data.scene.selected_animated_mesh_data) {
            MeshRenderData const & mesh_data = data.mesh_data;

            bind_transform_and_camera_data(
                *m_animated_selection_shader,
                mesh_data.transform,
                render_data.camera_data
            );
            bind_node_data(
                *m_animated_selection_shader,
                mesh_data.node_data
            );
            bind_bone_data(
                *m_animated_selection_shader,
                render_data.scene.bone_data[data.bone_data_index]
            );

            meshes.at(mesh_data.mesh_id).draw_elements_triangles();
        }
    }
}

void GLRenderer::create_canvas_geometry(
    std::vector<RenderRect2D> const & data
) {
    thread_local std::vector<CanvasGeometry> geometry;
    geometry.clear();

    for (RenderRect2D const & rect : data) {
        glm::vec2 pos = rect.position;
        glm::vec2 dim = rect.dimension;

        glm::vec4 v0{ pos.x,         pos.y,         rect.uv0.x, rect.uv0.y };
        glm::vec4 v1{ pos.x + dim.x, pos.y,         rect.uv1.x, rect.uv1.y };
        glm::vec4 v2{ pos.x + dim.x, pos.y + dim.y, rect.uv2.x, rect.uv2.y };
        glm::vec4 v3{ pos.x,         pos.y + dim.y, rect.uv3.x, rect.uv3.y };

        geometry.emplace_back(CanvasGeometry{ v0, rect.color });
        geometry.emplace_back(CanvasGeometry{ v1, rect.color });
        geometry.emplace_back(CanvasGeometry{ v2, rect.color });

        geometry.emplace_back(CanvasGeometry{ v0, rect.color });
        geometry.emplace_back(CanvasGeometry{ v2, rect.color });
        geometry.emplace_back(CanvasGeometry{ v3, rect.color });
    }

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_canvas_vbo));
    GL_CHECK(glBufferData(
        GL_ARRAY_BUFFER,
        geometry.size() * sizeof(geometry[0]),
        geometry.data(),
        GL_DYNAMIC_DRAW
    ));
}

void GLRenderer::draw_canvas_elements(
    size_t start,
    size_t end,
    GLuint texture_id
) {
    static const GLVarString tex_str = "u_Texture";
    GL_CHECK(glUniform1i(m_canvas_shader->get_uniform_loc(tex_str), 0));
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture_id));

    GL_CHECK(glBindVertexArray(m_canvas_vao));

    /* start, end is in render rect counts. We need to convert this to triangle
     * counts.
     */
    size_t buf_start = 6 * start;
    size_t buf_end = 6 * ((end - start) + 1);
    GL_CHECK(glDrawArrays(GL_TRIANGLES, buf_start, buf_end));
}

void GLRenderer::render_canvas(std::vector<RenderRect2D> & data) {
    if (data.empty()) return;

    GL_CHECK(glDepthMask(GL_FALSE));
    GL_CHECK(glDisable(GL_DEPTH_TEST));
    GL_CHECK(glDisable(GL_CULL_FACE));
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* The canvas will be rendered to whatever framebuffer that was already
     * bound.
     */
    GLint fbo = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo);

    GLenum attachment = fbo == 0 ? GL_BACK : GL_COLOR_ATTACHMENT0;

    GL_CHECK(glDrawBuffers(1, &attachment));
    GL_CHECK(glUseProgram(m_canvas_shader->shader()));

    /* sort canvas data */
    std::sort(data.begin(), data.end(),
    [] (RenderRect2D const & a, RenderRect2D const & b)
        {
            return (a.layer < b.layer);
        }
    );

    create_canvas_geometry(data);

    size_t group_start = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        if (i + 1 == data.size() || data[i].texture != data[i + 1].texture) {
            GLuint tex_id = data[i].texture == NO_RESOURCE ?
                m_texture_manager.texture_1x1_0xffffffff() :
                m_texture_manager.get_texture(data[i].texture);

            draw_canvas_elements(group_start, i, tex_id);
            group_start = i + 1;
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
    GL_CHECK(glUniform1i(s.get_uniform_loc(n_point_lights_str), static_cast<int>(light_data.number_of_point_lights)));

    static const GLVarString point_lights_0_pos_str = "u_PointLights[0].position";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(point_lights_0_pos_str), 1, &light_data.point_lights[0].position[0]));
    static const GLVarString point_lights_0_col_str = "u_PointLights[0].color";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(point_lights_0_col_str), 1, &light_data.point_lights[0].light.color[0]));
    static const GLVarString point_lights_0_a_str = "u_PointLights[0].a";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_0_a_str), light_data.point_lights[0].light.quadratic_term));
    static const GLVarString point_lights_0_b_str = "u_PointLights[0].b";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_0_b_str), light_data.point_lights[0].light.linear_term));
    static const GLVarString point_lights_0_c_str = "u_PointLights[0].c";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_0_c_str), light_data.point_lights[0].light.constant_term));

    static const GLVarString point_lights_1_pos_str = "u_PointLights[1].position";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(point_lights_1_pos_str), 1, &light_data.point_lights[1].position[0]));
    static const GLVarString point_lights_1_col_str = "u_PointLights[1].color";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(point_lights_1_col_str), 1, &light_data.point_lights[1].light.color[0]));
    static const GLVarString point_lights_1_a_str = "u_PointLights[1].a";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_1_a_str), light_data.point_lights[1].light.quadratic_term));
    static const GLVarString point_lights_1_b_str = "u_PointLights[1].b";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_1_b_str), light_data.point_lights[1].light.linear_term));
    static const GLVarString point_lights_1_c_str = "u_PointLights[1].c";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_1_c_str), light_data.point_lights[1].light.constant_term));

    static const GLVarString point_lights_2_pos_str = "u_PointLights[2].position";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(point_lights_2_pos_str), 1, &light_data.point_lights[2].position[0]));
    static const GLVarString point_lights_2_col_str = "u_PointLights[2].color";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(point_lights_2_col_str), 1, &light_data.point_lights[2].light.color[0]));
    static const GLVarString point_lights_2_a_str = "u_PointLights[2].a";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_2_a_str), light_data.point_lights[2].light.quadratic_term));
    static const GLVarString point_lights_2_b_str = "u_PointLights[2].b";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_2_b_str), light_data.point_lights[2].light.linear_term));
    static const GLVarString point_lights_2_c_str = "u_PointLights[2].c";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_2_c_str), light_data.point_lights[2].light.constant_term));

    static const GLVarString point_lights_3_pos_str = "u_PointLights[3].position";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(point_lights_3_pos_str), 1, &light_data.point_lights[3].position[0]));
    static const GLVarString point_lights_3_col_str = "u_PointLights[3].color";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(point_lights_3_col_str), 1, &light_data.point_lights[3].light.color[0]));
    static const GLVarString point_lights_3_a_str = "u_PointLights[3].a";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_3_a_str), light_data.point_lights[3].light.quadratic_term));
    static const GLVarString point_lights_3_b_str = "u_PointLights[3].b";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_3_b_str), light_data.point_lights[3].light.linear_term));
    static const GLVarString point_lights_3_c_str = "u_PointLights[3].c";
    GL_CHECK(glUniform1f(s.get_uniform_loc(point_lights_3_c_str), light_data.point_lights[3].light.constant_term));

    glm::vec3 dir_light_dir = glm::normalize(light_data.directional_light.direction);
    static const GLVarString dir_light_dir_str = "u_DirectionalLight.direction";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(dir_light_dir_str), 1, &dir_light_dir[0]));
    static const GLVarString dir_light_col_str = "u_DirectionalLight.color";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(dir_light_col_str), 1, &light_data.directional_light.color[0]));

    static const GLVarString dir_light_on_str = "u_DirectionalLightOn";
    GL_CHECK(glUniform1i(s.get_uniform_loc(dir_light_on_str), static_cast<int>(light_data.directional_light_on)));

    static const GLVarString ambient_light_str = "u_AmbientLight";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(ambient_light_str), 1, &light_data.ambient_light.color[0]));
}

void GLRenderer::bind_transform_and_camera_data(
    GLShader const & s,
    glm::mat4 const & transform,
    CameraRenderData const & data
) {
    glm::mat4 m_matrix = transform;
    glm::mat4 mv_matrix = data.view_matrix * transform;
    glm::mat4 vp_matrix = data.projection_matrix * data.view_matrix;
    glm::mat4 mvp_matrix = data.projection_matrix * mv_matrix;
    glm::mat3 inv_tpos_matrix = glm::inverse(glm::transpose(m_matrix));

    static const GLVarString view_pos_str = "u_ViewPosition";
    GL_CHECK(glUniform3fv(s.get_uniform_loc(view_pos_str), 1, &data.view_position[0]));

    static const GLVarString mmatrix_str = "u_MMatrix";
    GL_CHECK(glUniformMatrix4fv(s.get_uniform_loc(mmatrix_str), 1, GL_FALSE, &m_matrix[0][0]));
    static const GLVarString mvmatrix_str = "u_MVMatrix";
    static const GLVarString vpmatrix_str = "u_VPMatrix";
    GL_CHECK(glUniformMatrix4fv(s.get_uniform_loc(vpmatrix_str), 1, GL_FALSE, &vp_matrix[0][0]));
    GL_CHECK(glUniformMatrix4fv(s.get_uniform_loc(mvmatrix_str), 1, GL_FALSE, &mv_matrix[0][0]));
    static const GLVarString mvpmatrix_str = "u_MVPMatrix";
    GL_CHECK(glUniformMatrix4fv(s.get_uniform_loc(mvpmatrix_str), 1, GL_FALSE, &mvp_matrix[0][0]));
    static const GLVarString inv_tpos_matrix_str = "u_InvTposMMatrix";
    GL_CHECK(glUniformMatrix3fv(s.get_uniform_loc(inv_tpos_matrix_str), 1, GL_FALSE, &inv_tpos_matrix[0][0]));
}

void GLRenderer::bind_decal_data(
    GLShader const & s,
    CameraRenderData const & data
) {
    glm::mat4 inv_vp_matrix = glm::inverse(data.view_matrix) *
                              glm::inverse(data.projection_matrix);

    static const GLVarString inv_vp_str = "u_InvVP";
    GL_CHECK(glUniformMatrix4fv(s.get_uniform_loc(inv_vp_str), 1, GL_FALSE, &inv_vp_matrix[0][0]));

    static const GLVarString depth_map_str = "u_DepthMap";
    GL_CHECK(glUniform1i(s.get_uniform_loc(depth_map_str), 0));
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_source_buffers.depth_texture()));

    static const GLVarString normal_map_str = "u_NormalMap";
    GL_CHECK(glUniform1i(s.get_uniform_loc(normal_map_str), 1));
    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_source_buffers.normal_texture()));

    int w;
    int h;
    GL_CHECK(glfwGetWindowSize(m_window, &w, &h));
    GLint buffer_width = static_cast<GLint>(w / m_downscale_factor);
    GLint buffer_height = static_cast<GLint>(h / m_downscale_factor);

    static const GLVarString width_str = "u_BufferWidth";
    GL_CHECK(glUniform1i(s.get_uniform_loc(width_str), buffer_width));
    static const GLVarString height_str = "u_BufferHeight";
    GL_CHECK(glUniform1i(s.get_uniform_loc(height_str), buffer_height));


}

void GLRenderer::bind_node_data(
    GLShader const & shader,
    NodeData node_data
) {
    assert(node_data.id <= 0x00ffffffu || node_data.id == NO_NODE);

    uint32_t idu32;
    memcpy(&idu32, &node_data.id, sizeof(uint32_t));
    uint32_t packed_data = (idu32 & 0x00ffffffu) |
                           (node_data.selected ? 0xff000000u : 0x0u);

    static const GLVarString node_data_str = "u_NodeData";
    GLuint u_node_data = static_cast<GLuint>(packed_data);
    GL_CHECK(glUniform1ui(shader.get_uniform_loc(node_data_str), GLuint(u_node_data)));
}

void GLRenderer::bind_material_data(
    GLShader const & s,
    GLMaterial const & material,
    MaterialOverride const & mat_override
) {
    static const GLVarString albedo_map_str = "u_AlbedoMap";
    GL_CHECK(glUniform1i(s.get_uniform_loc(albedo_map_str), 0));
    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, material.albedo_map()));

    static const GLVarString normal_map_str = "u_NormalMap";
    GL_CHECK(glUniform1i(s.get_uniform_loc(normal_map_str), 1));
    GL_CHECK(glActiveTexture(GL_TEXTURE1));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, material.normal_map()));

    static const GLVarString metallic_map_str = "u_MetallicMap";
    GL_CHECK(glUniform1i(s.get_uniform_loc(metallic_map_str), 2));
    GL_CHECK(glActiveTexture(GL_TEXTURE2));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, material.metallic_map()));

    static const GLVarString roughness_map_str = "u_RoughnessMap";
    GL_CHECK(glUniform1i(s.get_uniform_loc(roughness_map_str), 3));
    GL_CHECK(glActiveTexture(GL_TEXTURE3));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, material.roughness_map()));

    glm::vec4 albedo = material.material().albedo;
    if (mat_override.tint_active) {
        albedo = albedo * mat_override.tint;
    }

    static const GLVarString albedo_str = "u_Albedo";
    GL_CHECK(glUniform4fv(s.get_uniform_loc(albedo_str), 1, &albedo[0]));
    static const GLVarString metallic_str = "u_Metallic";
    GL_CHECK(glUniform1f(s.get_uniform_loc(metallic_str), material.material().metallic));
    static const GLVarString roughness_str = "u_Roughness";
    GL_CHECK(glUniform1f(s.get_uniform_loc(roughness_str), material.material().roughness));
}

void GLRenderer::bind_bone_data(
    GLShader const & s,
    BoneData const & bone_data
) {
    static const GLVarString bones_str = "u_Bones";
    GL_CHECK(glUniformMatrix4fv(s.get_uniform_loc(bones_str), bone_data.bones.size(), GL_FALSE, &bone_data.bones[0][0][0]));
}

void GLRenderer::bind_texture(
    GLShader const & s,
    GLVarString const & uniform_str,
    unsigned int location,
    GLuint texture
) {
    glUniform1i(s.get_uniform_loc(uniform_str), location);
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + location));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
}
