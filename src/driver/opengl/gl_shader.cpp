#include "gl_shader.h"

#include "src/driver/opengl/gl_shader_utility.h"

using namespace prt3;

GLShader::GLShader(const char * vertex_path, const char * fragment_path) {
    m_shader = glshaderutility::create_shader(
        vertex_path,
        fragment_path
    );
}

GLint GLShader::get_uniform_loc(GLVarString const & uniform) const {
    auto search = m_uniform_cache.find(uniform);
    if (search == m_uniform_cache.end()) {
        GLint loc = glGetUniformLocation(m_shader, uniform.data());
        m_uniform_cache[uniform] = loc;
        return loc;
    }
    return search->second;
}

GLint GLShader::get_attrib_loc(GLVarString const & attrib) const {
    auto search = m_attrib_cache.find(attrib);
    if (search == m_attrib_cache.end()) {
        GLint loc = glGetAttribLocation(m_shader, attrib.data());
        m_attrib_cache[attrib] = loc;
        return loc;
    }
    return search->second;
}
