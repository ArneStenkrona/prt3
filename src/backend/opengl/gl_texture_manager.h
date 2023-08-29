#ifndef PRT3_GL_TEXTURE_MANAGER_H
#define PRT3_GL_TEXTURE_MANAGER_H

#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/texture.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace prt3 {

class GLTextureManager {
public:
    GLTextureManager();
    ~GLTextureManager();
    void init();

    ResourceID upload_texture(TextureData const & data);
    void free_texture(ResourceID id);

    GLuint get_texture(ResourceID id) { return m_textures.at(id); }

    GLuint texture_1x1_0xffffffff() const { return m_texture_1x1_0xffffffff; }
    GLuint texture_1x1_0x0000ff() const { return m_texture_1x1_0x0000ff; }
    GLuint texture_1x1_0xff() const { return m_texture_1x1_0xff; }
private:
    /* Defaults for materials without texture bindings */
    GLuint m_texture_1x1_0xffffffff;
    GLuint m_texture_1x1_0x0000ff;
    GLuint m_texture_1x1_0xff;

    std::unordered_map<ResourceID, GLuint> m_textures;
    std::unordered_map<GLuint, ResourceID> m_resource_ids;

    std::vector<ResourceID> m_free_ids;

    GLuint upload_texture(
        unsigned char * data,
        int w, int h,
        GLenum format,
        bool mipmaps
    );
};

}

#endif
