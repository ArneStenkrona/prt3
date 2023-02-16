#include "src/engine/core/engine.h"

#include "src/util/file_util.h"

#include <emscripten.h>

#include <functional>
#include <iostream>
#include <string>
#include <cstring>

prt3::Engine engine;
void main_loop() { engine.execute_frame(); }

struct Args {
    std::string project_path;
};

Args parse_args(int argc, char** argv) {
    Args args;

    if (argc <= 0) {
        return {};
    }
    for (int i = 0; i < argc; ++i) {
        char const * arg = argv[i];

        if (strstr(arg, "project=") != nullptr) {
            args.project_path = strchr(arg, '=') + 1;
        }
    }

    return args;
}

int main(int argc, char** argv) {
    Args args = parse_args(argc, argv);

    if (!args.project_path.empty()) {
        engine.set_project_from_path(args.project_path);
    }

    emscripten_set_main_loop(main_loop, 0, true);

    return EXIT_SUCCESS;
}
