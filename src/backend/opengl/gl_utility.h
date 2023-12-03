#ifndef PRT3_GL_UTILITY_H
#define PRT3_GL_UTILITY_H

#include "src/util/fixed_string.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

namespace prt3 {

typedef FixedString<64> GLVarString;

GLenum glCheckError_(const char * file, int line);

} // namespace prt3

#if DEBUG == 0
#else
#define PRT3GLDEBUG
#endif

#ifdef PRT3GLDEBUG
    #define GL_CHECK(stmt) do { \
            stmt; \
            glCheckError_(__FILE__, __LINE__); \
        } while (0)
#else
    #define GL_CHECK(stmt) stmt
#endif

#endif
