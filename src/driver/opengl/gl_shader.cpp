#include "gl_shader.h"

#include "src/driver/opengl/gl_shader_utility.h"

using namespace prt3;

GLShader::GLShader(const char * vertex_path, const char * fragment_path) {
    m_shader = glshaderutility::create_shader(
        vertex_path,
        fragment_path
    );
}

GLint GLShader::get_uniform_loc(UniformVarString const & uniform) const {
    auto search = m_uniform_cache.find(uniform);
    if (search == m_uniform_cache.end()) {
        GLint loc = glGetUniformLocation(m_shader, uniform.data());
        m_uniform_cache[uniform] = loc;
        return loc;
    }
    return search->second;
}
