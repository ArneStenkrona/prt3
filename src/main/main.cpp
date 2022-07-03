#include "src/engine/core/engine.h"

#include <emscripten.h>

#include <functional>
#include <iostream>

// the function called by the javascript code
// extern "C" void EMSCRIPTEN_KEEPALIVE toggle_background_color() { background_is_black = !background_is_black;

prt3::Engine engine;
void main_loop() { engine.execute_frame(); }

int main(int, char**)
{
    emscripten_set_main_loop(main_loop, 0, true);

    return EXIT_SUCCESS;
}
