#include "gl_source_buffers.h"

#include <cassert>

using namespace prt3;

void generate_fb_texture(
    GLuint & texture,
    GLint    width,
    GLint    height,
    GLint    internal_format,
    GLenum   /*format*/,
    GLenum   /*type*/,
    GLenum   attachment
) {
    GL_CHECK(glGenTextures(1, &texture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
    GL_CHECK(glTexStorage2D(
        GL_TEXTURE_2D,
        1,
        internal_format,
        width,
        height
    ));

    // glTexImage2D slower?
    // glTexImage2D(
    //     GL_TEXTURE_2D,
    //     0,
    //     internal_format,
    //     width,
    //     height,
    //     0,
    //     format,
    //     type,
    //     0
    // );

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GL_CHECK(glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        attachment,
        GL_TEXTURE_2D,
        texture,
        0
    ));
}

void GLSourceBuffers::init(GLint width, GLint height) {
    clean_up();

    GL_CHECK(glGenFramebuffers(1, &m_framebuffer));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer));

    generate_fb_texture(
        m_color_texture,
        width,
        height,
        GL_RGB8,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        color_attachment()
    );

    generate_fb_texture(
        m_normal_texture,
        width,
        height,
        GL_RGB8,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        normal_attachment()
    );

    generate_fb_texture(
        m_node_data_texture,
        width,
        height,
        GL_RGBA8, // GL_R32I?
        GL_RGBA, // GL_RED?
        GL_UNSIGNED_BYTE, // GL_INT?
        node_data_attachment()
    );

    generate_fb_texture(
        m_depth_texture,
        width,
        height,
        GL_DEPTH_COMPONENT24,
        GL_DEPTH_COMPONENT,
        GL_UNSIGNED_INT,
        GL_DEPTH_ATTACHMENT
    );

    GLenum fb_status;
    GL_CHECK(fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER));
    if(fb_status != GL_FRAMEBUFFER_COMPLETE) {
        assert(false && "Failed to create framebuffer!");
    }

    GLenum attachments[3] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
    };

    GL_CHECK(glDrawBuffers(3, attachments));

    // selection framebuffer
    GL_CHECK(glGenFramebuffers(1, &m_selection_framebuffer));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_selection_framebuffer));

    generate_fb_texture(
        m_selected_texture,
        width,
        height,
        GL_R8,
        GL_RED,
        GL_UNSIGNED_BYTE,
        selected_attachment()
    );

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        assert(false && "Failed to create selction framebuffer!");
    }

    GLenum selection_attachments[1] = {
        GL_COLOR_ATTACHMENT0
    };

    GL_CHECK(glDrawBuffers(1, selection_attachments));

    // uniforms
    m_uniform_names.emplace_back("u_ColorBuffer", m_color_texture);
    m_uniform_names.emplace_back("u_NormalBuffer", m_normal_texture);
    m_uniform_names.emplace_back("u_NodeDataBuffer", m_node_data_texture);
    m_uniform_names.emplace_back("u_SelectedBuffer", m_selected_texture);
    m_uniform_names.emplace_back("u_DepthBuffer", m_depth_texture);

    // transparency
    GL_CHECK(glGenFramebuffers(1, &m_accum_framebuffer));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_accum_framebuffer));

    generate_fb_texture(
        m_accum_texture,
        width,
        height,
        GL_RGBA16F,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        GL_COLOR_ATTACHMENT0
    );

    generate_fb_texture(
        m_accum_alpha_texture,
        width,
        height,
        GL_R16F,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        GL_COLOR_ATTACHMENT1
    );

    GL_CHECK(glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        m_depth_texture,
        0
    ));

    m_transparency_uniform_names.emplace_back("u_AccumBuffer", m_accum_texture);
    m_transparency_uniform_names.emplace_back("u_AccumAlphaBuffer", m_accum_alpha_texture);

    // decal
    GL_CHECK(glDrawBuffers(0, nullptr));

    GL_CHECK(glGenFramebuffers(1, &m_decal_framebuffer));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_decal_framebuffer));

    GL_CHECK(glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_color_texture,
        0
    ));
}

void GLSourceBuffers::clean_up() {
    if (m_framebuffer != 0) {
        GL_CHECK(glDeleteFramebuffers(1, &m_framebuffer));
        GL_CHECK(glDeleteTextures(1, &m_color_texture));
        GL_CHECK(glDeleteTextures(1, &m_normal_texture));
        GL_CHECK(glDeleteTextures(1, &m_node_data_texture));
        GL_CHECK(glDeleteTextures(1, &m_selected_texture));
        GL_CHECK(glDeleteTextures(1, &m_depth_texture));

        m_color_texture = 0;
        m_normal_texture = 0;
        m_node_data_texture = 0;
        m_selected_texture = 0;
        m_depth_texture = 0;

        m_uniform_names.resize(0);
    }

    if (m_selection_framebuffer != 0) {
        GL_CHECK(glDeleteFramebuffers(1, &m_selection_framebuffer));
    }

    if (m_accum_framebuffer != 0) {
        GL_CHECK(glDeleteFramebuffers(1, &m_accum_framebuffer));
        GL_CHECK(glDeleteTextures(1, &m_accum_texture));
        GL_CHECK(glDeleteTextures(1, &m_accum_alpha_texture));

        m_accum_texture = 0;
        m_accum_alpha_texture = 0;

        m_transparency_uniform_names.resize(0);
    }

    if (m_decal_framebuffer != 0) {
        GL_CHECK(glDeleteFramebuffers(1, &m_decal_framebuffer));
    }
}
