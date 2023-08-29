#include "gl_texture_manager.h"

#include "src/backend/opengl/gl_utility.h"
#include "src/util/log.h"

#include <cassert>

using namespace prt3;

GLTextureManager::GLTextureManager() {}

void GLTextureManager::init() {
    unsigned char data_0xffffffff[4] = { 0xff, 0xff, 0xff, 0xff };
    m_texture_1x1_0xffffffff = upload_texture(data_0xffffffff, 1, 1, GL_RGBA, false);
    unsigned char data_0x000000ff[3] = { 0x00, 0x00, 0xff };
    m_texture_1x1_0x0000ff = upload_texture(data_0x000000ff, 1, 1, GL_RGB, false);
    unsigned char data_0xff[3] = { 0xff };
    m_texture_1x1_0xff = upload_texture(data_0xff, 1, 1, GL_LUMINANCE, false);
}

GLTextureManager::~GLTextureManager() {
    for (auto const & pair : m_textures) {
        glDeleteTextures(1, &pair.second);
        glCheckError();
    }
    glDeleteTextures(1, &m_texture_1x1_0xffffffff);
    glCheckError();
    glDeleteTextures(1, &m_texture_1x1_0x0000ff);
    glCheckError();
    glDeleteTextures(1, &m_texture_1x1_0xff);
    glCheckError();
}

ResourceID GLTextureManager::upload_texture(TextureData const & data) {
    // TODO: proper format detection
    GLenum format = 0;
    switch (data.channels) {
        case 1: {
            format = GL_LUMINANCE;
            break;
        }
        case 2: {
            format = GL_LUMINANCE_ALPHA;
            break;
        }
        case 3: {
            format = GL_RGB;
            break;
        }
        case 4: {
            format = GL_RGBA;
            break;
        }
        default: {
            assert(false && "Invalid number of channels");
        }
    }

    GLuint texture_handle;
    glGenTextures(1, &texture_handle);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glCheckError();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glCheckError();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glCheckError();

    glTexImage2D(GL_TEXTURE_2D, 0, format, data.width, data.height, 0,
                 format, GL_UNSIGNED_BYTE, data.data);
    glCheckError();
    glGenerateMipmap(GL_TEXTURE_2D);
    glCheckError();

    ResourceID id;
    if (m_free_ids.empty()) {
        id = m_textures.size();
    } else {
        id = m_free_ids.back();
        m_free_ids.pop_back();
    }

    m_textures.emplace(
        std::make_pair(id, texture_handle)
    );
    m_resource_ids.emplace(
        std::make_pair(texture_handle, id)
    );

    return id;
}

GLuint GLTextureManager::upload_texture(
    unsigned char * data,
    int w, int h,
    GLenum format,
    bool mipmap
) {
    GLuint texture;
    glGenTextures(1, &texture);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, texture);
    glCheckError();
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
                    format, GL_UNSIGNED_BYTE, data);
    glCheckError();
    if (mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
        glCheckError();
    }
    return texture;
}

void GLTextureManager::free_texture(ResourceID id) {
    GLuint handle = m_textures.at(id);
    glDeleteTextures(1, &handle);
    glCheckError();
    m_textures.erase(id);
    m_resource_ids.erase(handle);
    m_free_ids.push_back(id);
}
