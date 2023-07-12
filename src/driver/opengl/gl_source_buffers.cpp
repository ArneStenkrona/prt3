#include "gl_source_buffers.h"

#include <cassert>

using namespace prt3;

void generate_texture(
    GLuint & texture,
    GLint    width,
    GLint    height,
    GLint    internal_format,
    GLenum   /*format*/,
    GLenum   /*type*/,
    GLenum   attachment
) {
    glGenTextures(1, &texture);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, texture);
    glCheckError();
    glTexStorage2D(
        GL_TEXTURE_2D,
        1,
        internal_format,
        width,
        height
    );
    glCheckError();

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
        attachment,
        GL_TEXTURE_2D,
        texture,
        0
    );
    glCheckError();
}

void GLSourceBuffers::init(GLint width, GLint height) {
    clean_up();

    glGenFramebuffers(1, &m_framebuffer);
    glCheckError();
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glCheckError();

    generate_texture(
        m_color_texture,
        width,
        height,
        GL_RGB8,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        color_attachment()
    );

    generate_texture(
        m_normal_texture,
        width,
        height,
        GL_RGB8,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        normal_attachment()
    );

    generate_texture(
        m_node_data_texture,
        width,
        height,
        GL_RGBA8, // GL_R32I?
        GL_RGBA, // GL_RED?
        GL_UNSIGNED_BYTE, // GL_INT?
        node_data_attachment()
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
    glCheckError();

    // selection framebuffer
    glGenFramebuffers(1, &m_selection_framebuffer);
    glCheckError();
    glBindFramebuffer(GL_FRAMEBUFFER, m_selection_framebuffer);
    glCheckError();

    generate_texture(
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

    glDrawBuffers(1, selection_attachments);
    glCheckError();

    // uniforms
    m_uniform_names.emplace_back("u_ColorBuffer", m_color_texture);
    m_uniform_names.emplace_back("u_NormalBuffer", m_normal_texture);
    m_uniform_names.emplace_back("u_NodeDataBuffer", m_node_data_texture);
    m_uniform_names.emplace_back("u_SelectedBuffer", m_selected_texture);
    m_uniform_names.emplace_back("u_DepthBuffer", m_depth_texture);


    // transparency
    glGenFramebuffers(1, &m_accum_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_accum_framebuffer);

    generate_texture(
        m_accum_texture,
        width,
        height,
        GL_RGBA16F,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        GL_COLOR_ATTACHMENT0
    );

    generate_texture(
        m_accum_alpha_texture,
        width,
        height,
        GL_R16F,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        GL_COLOR_ATTACHMENT1
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        m_depth_texture,
        0
    );

    m_transparency_uniform_names.emplace_back("u_AccumBuffer", m_accum_texture);
    m_transparency_uniform_names.emplace_back("u_AccumAlphaBuffer", m_accum_alpha_texture);
}

void GLSourceBuffers::clean_up() {
    if (m_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_framebuffer);
        glCheckError();
        glDeleteTextures(1, &m_color_texture);
        glCheckError();
        glDeleteTextures(1, &m_normal_texture);
        glCheckError();
        glDeleteTextures(1, &m_node_data_texture);
        glCheckError();
        glDeleteTextures(1, &m_selected_texture);
        glCheckError();
        glDeleteTextures(1, &m_depth_texture);
        glCheckError();

        m_color_texture = 0;
        m_normal_texture = 0;
        m_node_data_texture = 0;
        m_selected_texture = 0;
        m_depth_texture = 0;

        m_uniform_names.resize(0);
    }

    if (m_selection_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_selection_framebuffer);
        glCheckError();
    }

    if (m_accum_framebuffer != 0) {
        glDeleteFramebuffers(1, &m_accum_framebuffer);
        glCheckError();
        glDeleteTextures(1, &m_accum_texture);
        glCheckError();
        glDeleteTextures(1, &m_accum_alpha_texture);
        glCheckError();

        m_accum_texture = 0;
        m_accum_alpha_texture = 0;

        m_transparency_uniform_names.resize(0);
    }
}
