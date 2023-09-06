#include "gl_mesh.h"

#include "src/backend/opengl/gl_shader_utility.h"
#include "src/backend/opengl/gl_utility.h"
#include "src/util/log.h"

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

void GLMesh::draw_elements_triangles() const {
    glBindVertexArray(m_vao);
    glCheckError();
    glDrawElements(GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT,
                   reinterpret_cast<void*>(m_start_index * sizeof(GLuint)));
    glCheckError();
}

void GLMesh::draw_array_lines() const {
    glBindVertexArray(m_vao);
    glCheckError();
    glDrawArrays(GL_LINES, m_start_index, m_num_indices);
    glCheckError();
}

void GLMesh::draw_array_triangles() const {
    glBindVertexArray(m_vao);
    glCheckError();
    glDrawArrays(GL_TRIANGLES, m_start_index, m_num_indices);
    glCheckError();
}
