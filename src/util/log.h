#ifndef PRT3_CONSOLE_H
#define PRT3_CONSOLE_H

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else // __EMSCRIPTEN__
#include <cstdio>
#endif // __EMSCRIPTEN__

#ifdef __EMSCRIPTEN__
#define PRT3LOG(...) emscripten_log(EM_LOG_CONSOLE, __VA_ARGS__)
#define PRT3WARNING(...) emscripten_log(EM_LOG_WARN, __VA_ARGS__)
#define PRT3ERROR(...) emscripten_log(EM_LOG_ERROR, __VA_ARGS__)
#else // __EMSCRIPTEN__
#define PRT3LOG(...) printf(__VA_ARGS__)
#define PRT3WARNING(...) fprintf(stderr, __VA_ARGS__)
#define PRT3ERROR(...) fprintf(stderr, __VA_ARGS__)
#endif // __EMSCRIPTEN__


#endif // PRT3_CONSOLE_H
