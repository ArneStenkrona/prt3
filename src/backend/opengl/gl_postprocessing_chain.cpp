#include "gl_postprocessing_chain.h"

#include "src/backend/opengl/gl_utility.h"
#include "src/backend/opengl/gl_shader_utility.h"

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

    GL_CHECK(glGenVertexArrays(1, &m_screen_quad_vao));
    GL_CHECK(glBindVertexArray(m_screen_quad_vao));

    GL_CHECK(glGenBuffers(1, &m_screen_quad_vbo));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_screen_quad_vbo));

    GL_CHECK(glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(g_quad_vertex_buffer_data),
        g_quad_vertex_buffer_data,
        GL_STATIC_DRAW
    ));

    m_framebuffers.resize(chain.passes.size());
    m_color_textures.resize(chain.passes.size());
    m_color_textures[0] = source_buffers.color_texture();

    for (size_t i = 0; i < chain.passes.size() - 1; ++i) {
        int width = static_cast<int>(window_width / chain.passes[i].downscale_factor);
        int height = static_cast<int>(window_height / chain.passes[i].downscale_factor);

        GLuint & framebuffer = m_framebuffers[i];
        GL_CHECK(glGenFramebuffers(1, &framebuffer));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));

        GLuint & color_texture = m_color_textures[i+1];
        GL_CHECK(glGenTextures(1, &color_texture));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, color_texture));
        GL_CHECK(glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            width,
            height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            0
        ));

        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

        GL_CHECK(glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            color_texture,
            0
        ));

        GLenum fb_status;
        GL_CHECK(fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if(fb_status != GL_FRAMEBUFFER_COMPLETE) {
            assert(false && "Failed to create framebuffer!");
        }
    }
    m_framebuffers.back() = 0;

    for (size_t i = 0; i < m_framebuffers.size(); ++i) {
        int width = static_cast<int>(window_width / chain.passes[i].downscale_factor);
        int height = static_cast<int>(window_height / chain.passes[i].downscale_factor);

        GLuint shader = glshaderutility::create_shader(
            "assets/shaders/opengl/passthrough.vs",
            chain.passes[i].fragment_shader_path.c_str()
        );

        m_passes.emplace_back(
            shader,
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
    if (m_passes.empty()) {
        return;
    }

    for (GLPostProcessingPass & pass: m_passes) {
        GL_CHECK(glDeleteProgram(pass.shader().shader()));
    }

    if (m_framebuffers.size() == 1) {
        m_framebuffers.resize(0);
        m_color_textures.resize(0);
        return;
    }

    GL_CHECK(glDeleteFramebuffers(
        m_framebuffers.size() - 1,
        &m_framebuffers[0]
    ));

    GL_CHECK(glDeleteTextures(
        m_color_textures.size() - 1,
        &m_color_textures[1]
    ));

    m_framebuffers.resize(0);
    m_color_textures.resize(0);

    if (m_screen_quad_vbo == 0) {
        GL_CHECK(glDeleteBuffers(1, &m_screen_quad_vbo));
        GL_CHECK(glDeleteBuffers(1, &m_screen_quad_vao));
        m_screen_quad_vbo = 0;
        m_screen_quad_vao = 0;
    }
}
