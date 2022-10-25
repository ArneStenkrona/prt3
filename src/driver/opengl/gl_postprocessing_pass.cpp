#include "gl_postprocessing_pass.h"

#include "src/driver/opengl/gl_shader_utility.h"
#include "src/driver/opengl/gl_utility.h"

#include <SDL.h>

#include <iostream>

using namespace prt3;

GLPostProcessingPass::GLPostProcessingPass(
    const char * fragment_shader_path,
    unsigned int width,
    unsigned int height,
    GLSourceBuffers const & source_buffer,
    GLuint previous_color_buffer,
    GLuint target_framebuffer
)
 : m_shader{0},
   m_width{width},
   m_height{height},
   m_source_buffer{&source_buffer},
   m_previous_color_buffer{previous_color_buffer},
   m_target_framebuffer{target_framebuffer} {
    static const GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
    };

    glGenVertexArrays(1, &m_screen_quad_vao);
    glCheckError();
    glBindVertexArray(m_screen_quad_vao);
    glCheckError();

    glGenBuffers(1, &m_screen_quad_vbo);
    glCheckError();

    glBindBuffer(GL_ARRAY_BUFFER, m_screen_quad_vbo);
    glCheckError();

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(g_quad_vertex_buffer_data),
        g_quad_vertex_buffer_data,
        GL_STATIC_DRAW
    );
    glCheckError();

    set_shader(fragment_shader_path);
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

void GLPostProcessingPass::render(CameraRenderData const & camera_data) {
    GLint w = static_cast<GLint>(m_width);
    GLint h = static_cast<GLint>(m_height);

    glBindFramebuffer(GL_FRAMEBUFFER, m_target_framebuffer);
    glCheckError();
    glViewport(0, 0, w, h);
    glCheckError();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCheckError();

    glUseProgram(m_shader);
    glCheckError();

    GLenum tex_offset = 0;
    {
        GLint loc = glGetUniformLocation(m_shader, "u_PreviousColorBuffer");
        if (loc != -1) {
            glUniform1i(loc, tex_offset);
            glActiveTexture(GL_TEXTURE0 + tex_offset);
            glCheckError();
            glBindTexture(GL_TEXTURE_2D, m_previous_color_buffer);
            glCheckError();
            ++tex_offset;
        }
    }
    // for (UniformName const & uniform_name : m_source_buffer->uniform_names()) {
    //     GLint loc = glGetUniformLocation(m_shader, uniform_name.name.data());
    //     if (loc != -1) {
    //         glUniform1i(loc, tex_offset);
    //         glActiveTexture(GL_TEXTURE0 + tex_offset);
    //         glCheckError();
    //         glBindTexture(GL_TEXTURE_2D, uniform_name.value);
    //         glCheckError();
    //         ++tex_offset;
    //     }
    // }

    glshaderutility::set_vec3(m_shader, "u_ViewPosition", camera_data.view_position);
    glCheckError();
    glshaderutility::set_vec3(m_shader, "u_ViewDirection", camera_data.view_direction);
    glCheckError();

    glm::mat4 inv_v_matrix = glm::inverse(camera_data.view_matrix);
    glm::mat4 inv_p_matrix = glm::inverse(camera_data.projection_matrix);
    glshaderutility::set_mat4(m_shader, "u_InvVMatrix", inv_v_matrix);
    glCheckError();
    glshaderutility::set_mat4(m_shader, "u_InvPMatrix", inv_p_matrix);
    glCheckError();

    glshaderutility::set_float(m_shader, "u_NearPlane", camera_data.near_plane);
    glCheckError();
    glshaderutility::set_float(m_shader, "u_FarPlane", camera_data.far_plane);
    glCheckError();

    glshaderutility::set_float(m_shader, "u_PixelUnitX", 1.0f / w);
    glCheckError();
    glshaderutility::set_float(m_shader, "u_PixelUnitY", 1.0f / h);
    glCheckError();

    glBindVertexArray(m_screen_quad_vao);
    glCheckError();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glCheckError();
    glBindVertexArray(0);
    glCheckError();
}
