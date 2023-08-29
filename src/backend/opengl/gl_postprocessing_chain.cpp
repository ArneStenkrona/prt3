#include "gl_postprocessing_chain.h"

#include "src/backend/opengl/gl_utility.h"

using namespace prt3;

void GLPostProcessingChain::set_chain(
    PostProcessingChain const & chain,
    GLSourceBuffers const & source_buffers,
    int window_width,
    int window_height
) {
    clear_chain();
    if (chain.passes.empty()) {
        return;
    }

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

    m_framebuffers.resize(chain.passes.size());
    m_color_textures.resize(chain.passes.size());
    m_color_textures[0] = source_buffers.color_texture();

    for (size_t i = 0; i < chain.passes.size() - 1; ++i) {
        int width = static_cast<int>(window_width / chain.passes[i].downscale_factor);
        int height = static_cast<int>(window_height / chain.passes[i].downscale_factor);

        GLuint & framebuffer = m_framebuffers[i];
        glGenFramebuffers(1, &framebuffer);
        glCheckError();
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glCheckError();

        GLuint & color_texture = m_color_textures[i+1];
        glGenTextures(1, &color_texture);
        glCheckError();
        glBindTexture(GL_TEXTURE_2D, color_texture);
        glCheckError();
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            width,
            height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            0
        );
        glCheckError();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glCheckError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glCheckError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glCheckError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glCheckError();

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            color_texture,
            0
        );
        glCheckError();

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            assert(false && "Failed to create framebuffer!");
        }
    }
    m_framebuffers.back() = 0;

    for (size_t i = 0; i < m_framebuffers.size(); ++i) {
        int width = static_cast<int>(window_width / chain.passes[i].downscale_factor);
        int height = static_cast<int>(window_height / chain.passes[i].downscale_factor);
        m_passes.emplace_back(
            chain.passes[i].fragment_shader_path.c_str(),
            width,
            height,
            source_buffers,
            m_color_textures[i],
            m_framebuffers[i]
        );
    }
}

void GLPostProcessingChain::render(
    CameraRenderData const & camera_data,
    uint32_t frame
) {
    for (GLPostProcessingPass pass : m_passes) {
        pass.render(camera_data, frame, m_screen_quad_vao);
    }
}

void GLPostProcessingChain::clear_chain() {
    if (m_framebuffers.empty()) {
        return;
    }
    if (m_framebuffers.size() == 1) {
        m_framebuffers.resize(0);
        m_color_textures.resize(0);
        return;
    }
    glDeleteFramebuffers(m_framebuffers.size() - 1,
                         &m_framebuffers[0]);
    glDeleteTextures(m_color_textures.size() - 1,
                     &m_color_textures[1]);
    m_framebuffers.resize(0);
    m_color_textures.resize(0);

    if (m_screen_quad_vbo == 0) {
        glDeleteBuffers(1, &m_screen_quad_vbo);
        glCheckError();
        glDeleteBuffers(1, &m_screen_quad_vao);
        glCheckError();
        m_screen_quad_vbo = 0;
        m_screen_quad_vao = 0;
    }
}
