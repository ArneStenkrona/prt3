#ifndef PRT3_CONTEXT_H
#define PRT3_CONTEXT_H

#include "src/engine/audio/audio_manager.h"
#include "src/engine/scene/scene.h"
#include "src/engine/scene/scene_manager.h"
#include "src/engine/rendering/renderer.h"
#include "src/engine/rendering/material_manager.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/rendering/texture_manager.h"
#include "src/engine/core/backend_type.h"
#include "src/engine/core/input.h"
#include "src/engine/project/project.h"

namespace prt3 {

class Context {
public:
    Context(BackendType backend_type);
    Context(Context const & other) = delete;
    Context & operator=(Context const & other) = delete;

    ~Context();

    Renderer & renderer() { return m_renderer; }
    Input & input() { return m_renderer.input(); }
    MaterialManager & material_manager() { return m_material_manager; }
    ModelManager & model_manager() { return m_model_manager; }
    TextureManager & texture_manager() { return m_texture_manager; }
    Scene & edit_scene() { return m_edit_scene; }
    Scene const & edit_scene() const { return m_edit_scene; }
    Scene & game_scene() { return m_game_scene; }
    Scene const & game_scene() const { return m_game_scene; }
    SceneManager & scene_manager() { return m_scene_manager; }
    AudioManager & audio_manager() { return m_audio_manager; }
    Project & project() { return m_project; }

    void set_project_from_path(std::string const & path);

    void start_game(Scene const & scene);
    void end_game();

    TransitionState load_scene_if_queued(TransitionState state);

    void update_window_size(int w, int h);

    bool game_is_active() { return m_game_is_active; }

private:
    Renderer m_renderer;
    MaterialManager m_material_manager;
    ModelManager m_model_manager;
    TextureManager m_texture_manager;
    Scene m_edit_scene;
    Scene m_game_scene;
    SceneManager m_scene_manager;
    AudioManager m_audio_manager;
    Project m_project;

    bool m_game_is_active = false;
};

} // namespace prt3

#endif
