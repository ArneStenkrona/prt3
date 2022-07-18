#ifndef PRT3_GL_MESH_H
#define PRT3_GL_MESH_H

#include "src/driver/opengl/gl_material.h"
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
              uint32_t start_index,
              uint32_t num_indices);

    void draw(GLMaterial & material,
              SceneRenderData const & scene_data,
              MeshRenderData const & mesh_data);

private:
    bool m_initialized = false;

    /* render data */
    GLuint m_vao;

    uint32_t m_start_index;
    uint32_t m_num_indices;

    void setup_mesh();
};

} // namespace prt3

#endif
