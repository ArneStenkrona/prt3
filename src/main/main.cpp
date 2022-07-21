#include "src/engine/core/engine.h"

#include <emscripten.h>

#include <functional>
#include <iostream>

prt3::Engine engine;
void main_loop() { engine.execute_frame(); }

int main(int, char**)
{
    emscripten_set_main_loop(main_loop, 0, true);

    return EXIT_SUCCESS;
}
