#include "gl_utility.h"

#include <string>
#include <iostream>

GLenum prt3::glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case 1280:                  error = "INVALID_ENUM"; break;
            case 1281:                 error = "INVALID_VALUE"; break;
            case 1282:             error = "INVALID_OPERATION"; break;
            case 1283:                error = "STACK_OVERFLOW"; break;
            case 1284:               error = "STACK_UNDERFLOW"; break;
            case 1285:                 error = "OUT_OF_MEMORY"; break;
            case 1286: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
