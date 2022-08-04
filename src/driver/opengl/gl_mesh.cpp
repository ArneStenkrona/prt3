#include "gl_mesh.h"

#include "src/driver/opengl/gl_shader_utility.h"
#include "src/driver/opengl/gl_utility.h"

using namespace prt3;

GLMesh::GLMesh() {

}

void GLMesh::init(GLuint vao,
                  uint32_t start_index,
                  uint32_t num_indices) {
    assert(!m_initialized && "GL Mesh is already initialized!");

    m_vao = vao;
    m_start_index = start_index;
    m_num_indices = num_indices;

    m_initialized = true;
}

void GLMesh::draw(GLMaterial const & material,
                  SceneRenderData const & scene_data,
                  MeshRenderData const & mesh_data) const {
    glm::mat4 m_matrix = mesh_data.transform;
    glm::mat4 mv_matrix = scene_data.view_matrix * mesh_data.transform;
    glm::mat4 mvp_matrix = scene_data.projection_matrix * mv_matrix;
    glm::mat3 inv_tpos_matrix = glm::inverse(glm::transpose(m_matrix));

    glUniformMatrix4fv(material.mmatrix_loc(), 1, GL_FALSE, &m_matrix[0][0]);
    glCheckError();
    glUniformMatrix4fv(material.mvmatrix_loc(), 1, GL_FALSE, &mv_matrix[0][0]);
    glCheckError();
    glUniformMatrix4fv(material.mvpmatrix_loc(), 1, GL_FALSE, &mvp_matrix[0][0]);
    glCheckError();
    glUniformMatrix3fv(material.inv_tpos_matrix_loc(), 1, GL_FALSE, &inv_tpos_matrix[0][0]);
    glCheckError();

    glUniform1i(material.albedo_map_loc(), 0);
    glActiveTexture(GL_TEXTURE0);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.albedo_map());
    glCheckError();

    glUniform1i(material.normal_map_loc(), 1);
    glActiveTexture(GL_TEXTURE1);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.normal_map());
    glCheckError();

    glUniform1i(material.metallic_map_loc(), 2);
    glActiveTexture(GL_TEXTURE2);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.metallic_map());
    glCheckError();

    glUniform1i(material.roughness_map_loc(), 3);
    glActiveTexture(GL_TEXTURE3);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.roughness_map());
    glCheckError();

    glm::vec4 albedo = material.albedo();
    glUniform4fv(material.albedo_loc(), 1, &albedo[0]);
    glCheckError();
    glUniform1f(material.metallic_loc(), material.metallic());
    glCheckError();
    glUniform1f(material.roughness_loc(), material.roughness());
    glCheckError();

    // draw mesh
    glBindVertexArrayOES(m_vao);
    glCheckError();
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT,
                   reinterpret_cast<void*>(m_start_index * sizeof(GLuint)));
    glCheckError();
}
