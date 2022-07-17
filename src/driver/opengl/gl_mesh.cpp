#include "gl_mesh.h"

#include "src/driver/opengl/gl_utility.h"

#include "glm/gtx/string_cast.hpp"

using namespace prt3;

GLMesh::GLMesh() {

}

void GLMesh::init(GLuint vao,
                  uint32_t start_index,
                  uint32_t num_indices,
                  std::vector<GLTexture>  const & /*textures*/) {
    assert(!m_initialized && "GL Mesh is already initialized!");

    m_vao = vao;
    m_start_index = start_index;
    m_num_indices = num_indices;

    m_initialized = true;
}

void GLMesh::draw(GLMaterial & material,
                  SceneRenderData const & scene_data,
                  MeshRenderData const & mesh_data) {
    glm::mat4 m_matrix = mesh_data.transform;
    glm::mat4 mv_matrix = scene_data.view_matrix * mesh_data.transform;
    glm::mat4 mvp_matrix = scene_data.projection_matrix * mv_matrix;

    material.shader().setMat4("u_MMatrix", m_matrix);
    glCheckError();
    material.shader().setMat4("u_MVMatrix", mv_matrix);
    glCheckError();
    material.shader().setMat4("u_MVPMatrix", mvp_matrix);
    glCheckError();
    material.shader().setMat3("u_InvTposMMatrix", glm::inverse(glm::transpose(m_matrix)));
    glCheckError();

    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.albedo_map());
    glCheckError();

    // draw mesh
    glBindVertexArrayOES(m_vao);
    glCheckError();
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT,
                   reinterpret_cast<void*>(m_start_index * sizeof(GLuint)));
    glCheckError();
    glBindVertexArrayOES(0);
    glCheckError();
}
