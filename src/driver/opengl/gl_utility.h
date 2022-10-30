#ifndef PRT3_GL_UTILITY_H
#define PRT3_GL_UTILITY_H

#include "src/util/fixed_string.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

namespace prt3 {

typedef FixedString<64> UniformVarString;

GLenum glCheckError_(const char *file, int line);

} // namespace prt3
// #define glCheckError() glCheckError_(__FILE__, __LINE__)
#define glCheckError()

#endif
