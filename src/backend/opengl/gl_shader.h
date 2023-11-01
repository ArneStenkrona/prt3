#ifndef PRT3_GL_SHADER_H
#define PRT3_GL_SHADER_H

#include "src/backend/opengl/gl_utility.h"
#include "src/util/hash_util.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <unordered_map>

namespace prt3 {

class GLShader {
public:
    GLShader(GLuint shader);
    GLint shader() const { return m_shader; }

    GLint get_uniform_loc(GLVarString const & uniform) const;
    GLint get_attrib_loc(GLVarString const & attrib) const;
private:
    GLuint m_shader;
    mutable std::unordered_map<GLVarString, GLint> m_uniform_cache;
    mutable std::unordered_map<GLVarString, GLint> m_attrib_cache;
};

} // namespace prt3

#endif
