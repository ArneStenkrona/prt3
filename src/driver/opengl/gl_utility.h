#ifndef PRT3_GL_UTILITY_H
#define PRT3_GL_UTILITY_H

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>

namespace prt3 {

GLenum glCheckError_(const char *file, int line);

} // namespace prt3
#define glCheckError() glCheckError_(__FILE__, __LINE__)

#endif
