#ifndef PRT3_GL_TEXTURE_MANAGER_H
#define PRT3_GL_TEXTURE_MANAGER_H

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <string>
#include <unordered_map>

namespace prt3 {

class GLTextureManager {
public:
    GLTextureManager();
    void init();

    GLuint retrieve_texture(char const * path, GLuint default_texture);
    bool attempt_free_texture(GLuint handle);

    GLuint texture_1x1_0xffffffff() const {return m_texture_1x1_0xffffffff; }
    GLuint texture_1x1_0x0000ff() const {return m_texture_1x1_0x0000ff; }
    GLuint texture_1x1_0xff() const {return m_texture_1x1_0xff; }
private:
    /* Defaults for materials without texture bindings */
    GLuint m_texture_1x1_0xffffffff;
    GLuint m_texture_1x1_0x0000ff;
    GLuint m_texture_1x1_0xff;

    std::unordered_map<std::string, GLuint> m_textures;
    std::unordered_map<GLuint, std::string> m_texture_handle_to_path;

    void load_texture(char const * path);
    GLuint load_texture(unsigned char * data,
                        int w, int h,
                        GLenum format,
                        bool mipma);
};

}

#endif
