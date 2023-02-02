#ifndef PRT3_CONTEXT_H
#define PRT3_CONTEXT_H

#include "src/engine/scene/scene.h"
#include "src/engine/scene/scene_manager.h"
#include "src/engine/rendering/renderer.h"
#include "src/engine/rendering/material_manager.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/physics/physics_system.h"
#include "src/engine/core/input.h"

namespace prt3 {

class Context {
public:
    Context();
    Context(Context const & other) = delete;
    Context & operator=(Context const & other) = delete;

    ~Context();

    Renderer & renderer() { return m_renderer; }
    Input & input() { return m_renderer.input(); }
    MaterialManager & material_manager() { return m_material_manager; }
    ModelManager & model_manager() { return m_model_manager; }
    Scene & edit_scene() { return m_edit_scene; }
    Scene & game_scene() { return m_game_scene; }
    SceneManager & scene_manager() { return m_scene_manager; }

    void set_game_scene(Scene const & scene) { m_game_scene = scene; }

    void load_scene_if_queued();

private:
    Renderer m_renderer;
    MaterialManager m_material_manager;
    ModelManager m_model_manager;
    Scene m_edit_scene;
    Scene m_game_scene;
    SceneManager m_scene_manager;

    void update_window_size(int w, int h);

    friend class Renderer;
};

} // namespace prt3

#endif
