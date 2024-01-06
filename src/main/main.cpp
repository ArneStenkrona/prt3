#include "src/engine/core/engine.h"
#include "src/main/args.h"
#include "src/engine/audio/audio_manager.h"

#include "src/util/file_util.h"
#include "src/util/log.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif //  __EMSCRIPTEN__

#include <functional>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

prt3::Engine engine;
#ifdef __EMSCRIPTEN__
void main_loop() { engine.execute_frame(); }
#endif //  __EMSCRIPTEN__

void parse_args(int argc, char** argv) {
    prt3::Args & args = prt3::Args::instance();

    if (argc <= 0) {
        return;
    }

    for (int i = 0; i < argc; ++i) {
        char const * arg = argv[i];

        if (strstr(arg, "--project=") != nullptr) {
            args.m_project_path = strchr(arg, '=') + 1;
        }

        if (strstr(arg, "--force-cached") != nullptr) {
            char const * val = strchr(arg, '=') + 1;
            if (strcmp(val, "true") == 0 ||
                strcmp(val, "1") == 0) {
                args.m_force_cached = true;
            }
        }
    }
}

int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);

    parse_args(argc, argv);

    if (!prt3::Args::project_path().empty()) {
        engine.set_project_from_path(prt3::Args::project_path());
    }

    // init random
    srand(0);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
#else // __EMSCRIPTEN__
    while (engine.execute_frame()) {}
#endif //  __EMSCRIPTEN__

    return EXIT_SUCCESS;
}
