#include "gl_postprocessing_pass.h"

#include "src/driver/opengl/gl_shader_utility.h"
#include "src/driver/opengl/gl_utility.h"

#include <SDL.h>

using namespace prt3;

GLPostProcessingPass::GLPostProcessingPass()
 : m_shader{0} {
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

    set_shader("assets/shaders/opengl/passthrough.fs");
}

void GLPostProcessingPass::set_shader(const char * fragment_shader_path) {
    glDeleteProgram(m_shader);
    glCheckError();

    m_shader = glshaderutility::create_shader(
        "assets/shaders/opengl/passthrough.vs",
        fragment_shader_path
    );

    GLint pos_attr = glGetAttribLocation(m_shader, "a_Position");
    glCheckError();
    glEnableVertexAttribArray(pos_attr);
    glCheckError();
    glVertexAttribPointer(pos_attr, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float),
                          0);
    glCheckError();
}

void GLPostProcessingPass::render(int w, int h,
                                  SceneRenderData const & scene_data,
                                  GLuint render_texture,
                                  GLuint depth_texture) {
    /* Render to window */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCheckError();
    glViewport(0, 0, w, h);
    glCheckError();

    glUseProgram(m_shader);
    glCheckError();
    GLint render_loc = glGetUniformLocation(m_shader, "u_RenderTexture");
    GLenum tex_offset = 0;
    if (render_loc != -1) {
        glUniform1i(render_loc, tex_offset);
        glActiveTexture(GL_TEXTURE0 + tex_offset);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, render_texture);
        glCheckError();
        ++tex_offset;
    }

    GLint depth_loc = glGetUniformLocation(m_shader, "u_DepthBuffer");
    if (depth_loc != -1) {
        glUniform1i(depth_loc, tex_offset);
        glActiveTexture(GL_TEXTURE0 + tex_offset);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, depth_texture);
        glCheckError();
        ++tex_offset;
    }

    glshaderutility::set_float(m_shader, "u_NearPlane", scene_data.near_plane);
    glCheckError();
    glshaderutility::set_float(m_shader, "u_FarPlane", scene_data.far_plane);
    glCheckError();

    glshaderutility::set_float(m_shader, "u_PixelUnitX", 1.0f / w);
    glCheckError();
    glshaderutility::set_float(m_shader, "u_PixelUnitY", 1.0f / h);
    glCheckError();

    glBindVertexArrayOES(m_screen_quad_vao);
    glCheckError();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glCheckError();
    glBindVertexArrayOES(0);
    glCheckError();
}
