#ifndef PRT3_CONTEXT_H
#define PRT3_CONTEXT_H

#include "src/engine/scene/scene.h"
#include "src/engine/rendering/renderer.h"
#include "src/engine/rendering/model_manager.h"

namespace prt3 {

class Context {
public:
    Context();

    Renderer & renderer() { return m_renderer; }
    ModelManager & model_manager() { return m_model_manager; }
    Scene & current_scene() { return m_scene; }
private:
    Renderer m_renderer;
    ModelManager m_model_manager;
    Scene m_scene;
};

} // namespace prt3

#endif
