#ifndef PRT3_GL_SOURCE_BUFFERS
#define PRT3_GL_SOURCE_BUFFERS

#include "src/util/fixed_string.h"

#include <SDL.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <vector>

namespace prt3 {

typedef FixedString<64> UniformVarString;

struct UniformName {
    UniformName() {}
    UniformName(char const * c, GLuint v)
    : name{c}, value{v} {}

    UniformVarString name = "";
    GLuint value = 0;
};

class GLSourceBuffers {
public:
    void init(GLint width, GLint height);
    void clean_up();

    GLuint framebuffer()      const { return m_framebuffer;      }
    GLuint color_texture()    const { return m_color_texture;    }
    GLuint normal_texture()   const { return m_normal_texture;   }
    GLuint id_texture()       const { return m_id_texture;       }
    GLuint selected_texture() const { return m_selected_texture; }
    GLuint depth_texture()    const { return m_depth_texture;    }

    std::vector<UniformName> const & uniform_names() const
    { return m_uniform_names; }
private:

    GLuint m_framebuffer;
    GLuint m_color_texture;
    GLuint m_normal_texture;
    GLuint m_id_texture;
    GLuint m_selected_texture;
    GLuint m_depth_texture;

    std::vector<UniformName> m_uniform_names;
};

} // namespace prt3

#endif
