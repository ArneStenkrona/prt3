#ifndef PRT3_GL_MESH_H
#define PRT3_GL_MESH_H

#include "src/driver/opengl/gl_material.h"
#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/postprocessing_chain.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <cstdint>
#include <vector>
#include <string>

namespace prt3 {

class GLMesh {
public:
    GLMesh();

    void init(
        GLuint vao,
        uint32_t start_index,
        uint32_t num_indices
    );

    void draw_elements_triangles() const;
    void draw_array_lines() const;
    void draw_array_triangles() const;

private:
    bool m_initialized = false;

    /* render data */
    GLuint m_vao;

    uint32_t m_start_index;
    uint32_t m_num_indices;
};

} // namespace prt3

#endif
