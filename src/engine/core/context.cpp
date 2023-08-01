#include "context.h"

#include "src/engine/component/script/script.h"
#include "src/engine/component/script_set.h"
#include "src/util/mem.h"
#include "src/util/log.h"

#include <fstream>
#include <unordered_set>

using namespace prt3;

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

Context::Context()
 : m_renderer{*this, 960, 540, 1.0f},
   m_material_manager{*this},
   m_model_manager{*this},
   m_edit_scene{*this},
   m_game_scene{*this},
   m_scene_manager{*this} {

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
