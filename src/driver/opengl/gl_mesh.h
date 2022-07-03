#ifndef PRT3_GL_MESH_H
#define PRT3_GL_MESH_H

#include "src/driver/opengl/gl_material.h"
#include "src/driver/opengl/gl_texture.h"
#include "src/engine/rendering/render_data.h"

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>

#include <cstdint>
#include <vector>
#include <string>

namespace prt3 {

class GLMesh {
public:
    GLMesh();

    void init(GLuint vao,
              GLuint vbo,
              std::vector<uint32_t> const & indices,
              std::vector<GLTexture>  const & textures);

    void draw(GLMaterial & material,
              SceneRenderData const & scene_data,
              MeshRenderData const & mesh_data);

private:
    bool m_initialized = false;

    /* render data */
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;

    uint32_t m_index_buffer_length;

    void setup_mesh();
};

} // namespace prt3

#endif
