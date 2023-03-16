#include "gl_texture_manager.h"

#include "src/util/log.h"

#include <SDL_image.h>

#include <cassert>

using namespace prt3;

GLTextureManager::GLTextureManager() {}

void GLTextureManager::init() {

    unsigned char data_0xffffffff[4] = { 0xff, 0xff, 0xff, 0xff };
    m_texture_1x1_0xffffffff = load_texture(data_0xffffffff, 1, 1, GL_RGBA,
                                            false);
    unsigned char data_0x000000ff[3] = { 0x00, 0x00, 0xff };
    m_texture_1x1_0x0000ff = load_texture(data_0x000000ff, 1, 1, GL_RGB,
                                          false);
    unsigned char data_0xff[3] = { 0xff };
    m_texture_1x1_0xff = load_texture(data_0xff, 1, 1, GL_LUMINANCE,
                                      false);
}

GLuint GLTextureManager::retrieve_texture(char const * path, GLuint default_texture) {
    std::string path_str{path};
    if (m_textures.find(path_str) == m_textures.end()) {
        load_texture(path);
    }
    if (m_textures.find(path_str) != m_textures.end()) {
        return m_textures[path_str];
    }
    return default_texture;
}

void GLTextureManager::load_texture(char const * path) {
    if (path[0] != '\0') {
        SDL_Surface * image = IMG_Load(path);

        if (image) {
            GLuint texture_handle;
            // TODO: proper format detection
            GLenum format = 0;
            switch (image->format->BytesPerPixel) {
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

            glGenTextures(1, &texture_handle);
            glBindTexture(GL_TEXTURE_2D, texture_handle);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            glTexImage2D(GL_TEXTURE_2D, 0, format, image->w, image->h, 0,
                         format, GL_UNSIGNED_BYTE, image->pixels);
            glGenerateMipmap(GL_TEXTURE_2D);

            SDL_FreeSurface(image);

            m_textures.emplace(
                std::make_pair(path, texture_handle)
            );
            m_texture_handle_to_path.emplace(
                std::make_pair(texture_handle, path)
            );
        } else {
            // TODO: proper error handling
            PRT3ERROR("failed to load tecture at path \"%s\".\n", path);
            PRT3ERROR("IMG_Load: %s.\n", IMG_GetError());
        }
    } else {
        // No texture path
    }
}

GLuint GLTextureManager::load_texture(unsigned char * data,
                                      int w, int h,
                                      GLenum format,
                                      bool mipmap) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0,
                    format, GL_UNSIGNED_BYTE, data);
    if (mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    return texture;
}

bool GLTextureManager::attempt_free_texture(GLuint handle) {
    if (m_texture_handle_to_path.find(handle) !=
        m_texture_handle_to_path.end()) {
        std::string const & path = m_texture_handle_to_path.at(handle);
        glDeleteTextures(1, &handle);
        m_textures.erase(path);
        m_texture_handle_to_path.erase(handle);
        return true;
    }
    return false;
}
