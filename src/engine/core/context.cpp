#include "context.h"

#include "src/engine/component/script/script.h"
#include "src/engine/component/script_set.h"
#include "src/util/mem.h"
#include "src/util/log.h"

#include <fstream>
#include <unordered_set>

using namespace prt3;

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

Context::Context(BackendType backend_type)
 : m_renderer{*this, 960, 540, 1.0f, backend_type},
   m_material_manager{*this},
   m_model_manager{*this},
   m_texture_manager{*this},
   m_edit_scene{*this},
   m_game_scene{*this},
   m_scene_manager{*this},
   m_audio_manager{backend_type} {

}

Context::~Context() {
    m_edit_scene.clear();
    m_game_scene.clear();
}

void Context::set_project_from_path(std::string const & path) {
    std::ifstream in(path, std::ios::binary);
    m_project.deserialize(in);
    in.close();

    if (!m_project.main_scene_path().empty()) {
        std::ifstream scene_in(m_project.main_scene_path(), std::ios::binary);
        m_edit_scene.deserialize(scene_in);
        scene_in.close();
    }
}

TransitionState Context::load_scene_if_queued(TransitionState state) {
    return m_scene_manager.load_scene_if_queued(
        state,
        m_game_scene,
        m_edit_scene
    );
}

void Context::start_game(Scene const & scene) {
    m_game_scene = scene;
    m_game_is_active = true;
    m_project.on_game_start(m_game_scene);
}

void Context::end_game() {
    m_project.on_game_end(m_game_scene);
    m_game_is_active = false;
}
