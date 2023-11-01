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
    m_vao = vao;
    m_start_index = start_index;
    m_num_indices = num_indices;

    m_initialized = true;
}

void GLMesh::draw_elements_triangles() const {
    GL_CHECK(glBindVertexArray(m_vao));
    GL_CHECK(glDrawElements(
        GL_TRIANGLES, m_num_indices, GL_UNSIGNED_INT,
        reinterpret_cast<void*>(m_start_index * sizeof(GLuint))
    ));
}

void GLMesh::draw_array_lines() const {
    GL_CHECK(glBindVertexArray(m_vao));
    GL_CHECK(glDrawArrays(GL_LINES, m_start_index, m_num_indices));
}

void GLMesh::draw_array_triangles() const {
    GL_CHECK(glBindVertexArray(m_vao));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, m_start_index, m_num_indices));
}

