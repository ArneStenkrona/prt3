#include "engine.h"

using namespace prt3;

Engine::Engine() {

}

void Engine::execute_frame() {
    // TODO: measure real delta time
    float fixed_delta_time = 1.0f / 60.0f;
    m_context.input().update();
    m_context.current_scene().update(fixed_delta_time);
}
