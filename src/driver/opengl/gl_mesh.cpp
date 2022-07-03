#include "gl_mesh.h"

using namespace prt3;

GLMesh::GLMesh() {

}

void GLMesh::init(GLuint vao,
                  GLuint vbo,
                  std::vector<uint32_t> const & indices,
                  std::vector<GLTexture>  const & /*textures*/) {
    assert(!m_initialized && "GL Mesh is already initialized!");

    m_vao = vao;
    m_vbo = vbo;
    m_index_buffer_length = indices.size();

    glGenBuffers(1, &m_ebo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t),
                 &indices[0], GL_STATIC_DRAW);

    m_initialized = true;
}

void GLMesh::draw(GLMaterial & material,
                  SceneRenderData const & scene_data,
                  MeshRenderData const & mesh_data) {
    glm::mat4 mv_matrix = scene_data.view_matrix * mesh_data.transform;
    glm::mat4 mvp_matrix = scene_data.projection_matrix * mv_matrix;

    material.shader().use();
    material.shader().setMat4("u_MVMatrix", mv_matrix);
    material.shader().setMat4("u_MVPMatrix", mvp_matrix);

    // draw mesh
    glBindVertexArrayOES(m_vao);
    glDrawElements(GL_TRIANGLES, m_index_buffer_length, GL_UNSIGNED_INT, 0);
    glBindVertexArrayOES(0);
}
