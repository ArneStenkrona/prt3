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
        GL_CHECK(glDeleteTextures(1, &pair.second));
    }
    GL_CHECK(glDeleteTextures(1, &m_texture_1x1_0xffffffff));
    GL_CHECK(glDeleteTextures(1, &m_texture_1x1_0x0000ff));
    GL_CHECK(glDeleteTextures(1, &m_texture_1x1_0xff));
}

ResourceID GLTextureManager::upload_texture(TextureData const & data) {
    // TODO: proper format detection
    GLenum format = 0;
    GLint alignment = 0;
    switch (data.channels) {
        case 1: {
            format = GL_LUMINANCE;
            alignment = 1;
            break;
        }
        case 2: {
            format = GL_LUMINANCE_ALPHA;
            alignment = 2;
            break;
        }
        case 3: {
            format = GL_RGB;
            alignment = 1;
            break;
        }
        case 4: {
            format = GL_RGBA;
            alignment = 4;
            break;
        }
        default: {
            assert(false && "Invalid number of channels");
        }
    }
    GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, alignment));

    GLuint texture_handle;
    GL_CHECK(glGenTextures(1, &texture_handle));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture_handle));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    GL_CHECK(glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        data.width,
        data.height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data.data
    ));
    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

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

    TextureMetadata metadata;
    metadata.width = data.width;
    metadata.height = data.height;
    metadata.channels = data.channels;
    m_texture_metadata.emplace(
        std::make_pair(id, metadata)
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
    GL_CHECK(glGenTextures(1, &texture));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, texture));
    GL_CHECK(glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        w,
        h,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    ));
    if (mipmap) {
        GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
    }
    return texture;
}

void GLTextureManager::free_texture(ResourceID id) {
    GLuint handle = m_textures.at(id);
    GL_CHECK(glDeleteTextures(1, &handle));
    m_textures.erase(id);
    m_texture_metadata.erase(id);
    m_resource_ids.erase(handle);
    m_free_ids.push_back(id);
}
