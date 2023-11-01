#include "gl_shader.h"

#include "src/backend/opengl/gl_shader_utility.h"

using namespace prt3;

GLShader::GLShader(GLuint shader)
 : m_shader{shader} {}

GLint GLShader::get_uniform_loc(GLVarString const & uniform) const {
    auto search = m_uniform_cache.find(uniform);
    if (search == m_uniform_cache.end()) {
        GLint loc;
        GL_CHECK(loc = glGetUniformLocation(m_shader, uniform.data()));
        m_uniform_cache[uniform] = loc;
        return loc;
    }
    return search->second;
}

GLint GLShader::get_attrib_loc(GLVarString const & attrib) const {
    auto search = m_attrib_cache.find(attrib);
    if (search == m_attrib_cache.end()) {
        GLint loc;
        GL_CHECK(loc = glGetAttribLocation(m_shader, attrib.data()));
        m_attrib_cache[attrib] = loc;
        return loc;
    }
    return search->second;
}
