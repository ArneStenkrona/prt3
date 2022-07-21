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

void GLPostProcessingPass::render(int w, int h, GLuint render_texture) {
    /* Render to window */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCheckError();
    glViewport(0, 0, w, h);
    glCheckError();

    glUseProgram(m_shader);
    glCheckError();
    GLint render_loc = glGetUniformLocation(m_shader, "u_RenderTexture");
    glUniform1i(render_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, render_texture);
    glCheckError();

    glBindVertexArrayOES(m_screen_quad_vao);
    glCheckError();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glCheckError();
    glBindVertexArrayOES(0);
    glCheckError();
}
