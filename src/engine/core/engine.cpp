#include "engine.h"

using namespace prt3;

Engine::Engine()
: m_exit{false}
{

}

void Engine::run() {
    while (!m_exit) {
        m_context.current_scene().render();
    }
}
