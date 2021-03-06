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

    glshaderutility::set_mat4(material.shader(), "u_MMatrix", m_matrix);
    glCheckError();
    glshaderutility::set_mat4(material.shader(), "u_MVMatrix", mv_matrix);
    glCheckError();
    glshaderutility::set_mat4(material.shader(), "u_MVPMatrix", mvp_matrix);
    glCheckError();
    glshaderutility::set_mat3(material.shader(), "u_InvTposMMatrix", glm::inverse(glm::transpose(m_matrix)));
    glCheckError();

    GLint albedo_loc = glGetUniformLocation(material.shader(), "u_AlbedoMap");
    glUniform1i(albedo_loc, 0);
    glActiveTexture(GL_TEXTURE0);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.albedo_map());
    glCheckError();

    GLint normal_loc = glGetUniformLocation(material.shader(), "u_NormalMap");
    glUniform1i(normal_loc, 1);
    glActiveTexture(GL_TEXTURE1);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.normal_map());
    glCheckError();

    GLint metallic_loc = glGetUniformLocation(material.shader(), "u_MetallicMap");
    glUniform1i(metallic_loc, 2);
    glActiveTexture(GL_TEXTURE2);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.metallic_map());
    glCheckError();

    GLint roughness_loc = glGetUniformLocation(material.shader(), "u_RoughnessMap");
    glUniform1i(roughness_loc, 3);
    glActiveTexture(GL_TEXTURE3);
    glCheckError();
    glBindTexture(GL_TEXTURE_2D, material.roughness_map());
    glCheckError();

    glshaderutility::set_vec4(material.shader(), "u_Albedo", material.albedo());
    glCheckError();
    glshaderutility::set_float(material.shader(), "u_Metallic", material.metallic());
    glCheckError();
    glshaderutility::set_float(material.shader(), "u_Roughness", material.roughness());
    glCheckError();

    // draw mesh
    glBindVertexArrayOES(m_vao);
    glCheckError();
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT,
                   reinterpret_cast<void*>(m_start_index * sizeof(GLuint)));
    glCheckError();
}
