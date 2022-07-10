#include "gl_material.h"

#include <SDL_image.h>

using namespace prt3;

GLMaterial::GLMaterial(char const * vertex_shader_path,
                       char const * fragment_shader_path,
                       char const * albedo_path)
 : m_shader{vertex_shader_path, fragment_shader_path} {

    if (albedo_path[0] != '\0') {
        SDL_Surface * image = IMG_Load(albedo_path);

        // TODO: proper format detection
        GLenum format = GL_RGB;
        if(image->format->BytesPerPixel == 4) {
            format = GL_RGBA;
        }

        if (image) {
            glGenTextures(1, &m_texture);
            glBindTexture(GL_TEXTURE_2D, m_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, format, image->w, image->h, 0,
                        format, GL_UNSIGNED_BYTE, image->pixels);
            glGenerateMipmap(GL_TEXTURE_2D);

            SDL_FreeSurface(image);
        } else {
            // TODO: proper error handling
            std::cout << "failed to load texture at path \"" << albedo_path << "\"." << std::endl;
            std::cout << "IMG_Load: " << IMG_GetError() << std::endl;
        }
    } else {
        // No texture path, TODO: add default texture
    }
}
