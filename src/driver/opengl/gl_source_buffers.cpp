#include "gl_source_buffers.h"

#include <cassert>

using namespace prt3;

void generate_texture(
    GLuint & texture,
    GLint    width,
    GLint    height,
    GLint    internal_format,
    GLenum   format,
    GLenum   type,
    GLenum   attachment
) {
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        internal_format,
        width,
        height,
        0,
        format,
        type,
        0
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        attachment,
        GL_TEXTURE_2D,
        texture,
        0
    );
}

void GLSourceBuffers::init(GLint width, GLint height) {
    clean_up();

    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    generate_texture(
        m_color_texture,
        width,
        height,
        GL_RGB8,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        GL_COLOR_ATTACHMENT0
    );

    generate_texture(
        m_normal_texture,
        width,
        height,
        GL_RGB8,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        GL_COLOR_ATTACHMENT1
    );

    generate_texture(
        m_id_texture,
        width,
        height,
        GL_RGBA8, // GL_R32I?
        GL_RGBA, // GL_RED?
        GL_UNSIGNED_BYTE, // GL_INT?
        GL_COLOR_ATTACHMENT2
    );

    generate_texture(
        m_depth_texture,
        width,
        height,
        GL_DEPTH_COMPONENT24,
        GL_DEPTH_COMPONENT,
        GL_UNSIGNED_INT,
        GL_DEPTH_ATTACHMENT
    );

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        assert(false && "Failed to create framebuffer!");
    }

    GLenum attachments[3] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
    };

    glDrawBuffers(3, attachments);

    // selection framebuffer

    glGenFramebuffers(1, &m_selection_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_selection_framebuffer);

    generate_texture(
        m_selected_texture,
        width,
        height,
        GL_R8,
        GL_RED,
        GL_UNSIGNED_BYTE,
        GL_COLOR_ATTACHMENT0
    );

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        assert(false && "Failed to create selction framebuffer!");
    }

    GLenum selection_attachments[1] = {
        GL_COLOR_ATTACHMENT0
    };

    glDrawBuffers(1, selection_attachments);

    // uniforms
    m_uniform_names.emplace_back("u_ColorBuffer", m_color_texture);
    m_uniform_names.emplace_back("u_NormalBuffer", m_normal_texture);
    m_uniform_names.emplace_back("u_IDBuffer", m_id_texture);
    m_uniform_names.emplace_back("u_SelectedBuffer", m_selected_texture);
    m_uniform_names.emplace_back("u_DepthBuffer", m_depth_texture);
}

void GLSourceBuffers::clean_up() {
    if (m_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_framebuffer);
        glDeleteTextures(1, &m_color_texture);
        glDeleteTextures(1, &m_normal_texture);
        glDeleteTextures(1, &m_id_texture);
        glDeleteTextures(1, &m_selected_texture);
        glDeleteTextures(1, &m_depth_texture);

        m_color_texture = 0;
        m_normal_texture = 0;
        m_id_texture = 0;
        m_selected_texture = 0;
        m_depth_texture = 0;

        m_uniform_names.resize(0);
    }
}
