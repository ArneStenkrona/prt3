#include "gl_renderer.h"

#include "src/driver/opengl/gl_utility.h"

#include <vector>
#include <unordered_map>
#include <cassert>

using namespace prt3;

GLRenderer::GLRenderer(SDL_Window * window)
 : m_window{window},
   m_material_manager{m_texture_manager},
   m_model_manager{m_material_manager},
   m_passthrough_shader{"assets/shaders/opengl/passthrough.vs",
                        "assets/shaders/opengl/passthrough.fs"}
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
    glCheckError();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glCheckError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glCheckError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glCheckError();

    glGenRenderbuffers(1, &m_depth_buffer);
    glCheckError();
    glBindRenderbuffer(GL_RENDERBUFFER, m_depth_buffer);
    glCheckError();
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
    glCheckError();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                              GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER,
                              m_depth_buffer);
    glCheckError();

    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           m_render_texture,
                           0);
    glCheckError();

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        assert(false && "Failed to create framebuffer!");
    }

    /* Create objects for framebuffer post-processing and display */
    static const GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };

    glGenVertexArraysOES(1, &m_screen_quad_vao);
    glCheckError();
    glBindVertexArrayOES(m_screen_quad_vao);
    glCheckError();

    glGenBuffers(1, &m_screen_quad_vbo);
    glCheckError();

    glBindBuffer(GL_ARRAY_BUFFER, m_screen_quad_vbo);
    glCheckError();

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(g_quad_vertex_buffer_data),
                 g_quad_vertex_buffer_data,
                 GL_STATIC_DRAW);
    glCheckError();
    GLint pos_attr = glGetAttribLocation(m_passthrough_shader.ID, "a_Position");
    glCheckError();
    glEnableVertexAttribArray(pos_attr);
    glCheckError();
    glVertexAttribPointer(pos_attr, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float),
                          0);
    glCheckError();
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
        material.shader().use();
        glCheckError();
        material.shader().setVec3("u_ViewPosition", render_data.scene_data.view_position);
        glCheckError();
        // Light data
        LightRenderData const & light_data = render_data.light_data;
        material.shader().setInt("u_NumberOfPointLights", static_cast<int>(light_data.number_of_point_lights));

        material.shader().setVec3("u_PointLights[0].position", light_data.point_lights[0].position);
        material.shader().setVec3("u_PointLights[0].color", light_data.point_lights[0].light.color);
        material.shader().setFloat("u_PointLights[0].a", light_data.point_lights[0].light.quadratic_term);
        material.shader().setFloat("u_PointLights[0].b", light_data.point_lights[0].light.linear_term);
        material.shader().setFloat("u_PointLights[0].c", light_data.point_lights[0].light.constant_term);

        material.shader().setVec3("u_PointLights[1].position", light_data.point_lights[1].position);
        material.shader().setVec3("u_PointLights[1].color", light_data.point_lights[1].light.color);
        material.shader().setFloat("u_PointLights[1].a", light_data.point_lights[1].light.quadratic_term);
        material.shader().setFloat("u_PointLights[1].b", light_data.point_lights[1].light.linear_term);
        material.shader().setFloat("u_PointLights[1].c", light_data.point_lights[1].light.constant_term);

        material.shader().setVec3("u_PointLights[2].position", light_data.point_lights[2].position);
        material.shader().setVec3("u_PointLights[2].color", light_data.point_lights[2].light.color);
        material.shader().setFloat("u_PointLights[2].a", light_data.point_lights[2].light.quadratic_term);
        material.shader().setFloat("u_PointLights[2].b", light_data.point_lights[2].light.linear_term);
        material.shader().setFloat("u_PointLights[2].c", light_data.point_lights[2].light.constant_term);

        material.shader().setVec3("u_PointLights[3].position", light_data.point_lights[3].position);
        material.shader().setVec3("u_PointLights[3].color", light_data.point_lights[3].light.color);
        material.shader().setFloat("u_PointLights[3].a", light_data.point_lights[3].light.quadratic_term);
        material.shader().setFloat("u_PointLights[3].b", light_data.point_lights[3].light.linear_term);
        material.shader().setFloat("u_PointLights[3].c", light_data.point_lights[3].light.constant_term);
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

    /* Render to window */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCheckError();
    glViewport(0, 0, w, h);
    glCheckError();

    m_passthrough_shader.use();
    GLint render_loc = glGetUniformLocation(m_passthrough_shader.ID, "u_RenderTexture");
    glUniform1i(render_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, m_render_texture);
    glCheckError();

    glBindVertexArrayOES(m_screen_quad_vao);
    glCheckError();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glCheckError();
    glBindVertexArrayOES(0);
    glCheckError();

    SDL_GL_SwapWindow(m_window);
    glCheckError();

}
