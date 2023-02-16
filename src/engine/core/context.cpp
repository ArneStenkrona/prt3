#include "context.h"

#include "src/engine/component/script/script.h"
#include "src/engine/component/script_set.h"

#include <iostream>
#include <fstream>
#include <unordered_set>

using namespace prt3;

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

void Context::load_scene_if_queued() {
    if (m_scene_manager.scene_queued()) {
        thread_local std::unordered_set<ModelHandle> models_to_keep;
        thread_local std::unordered_set<ModelHandle> models_to_delete;

        models_to_keep.insert(
            m_edit_scene.referenced_models().begin(),
            m_edit_scene.referenced_models().end()
        );

        for (ModelHandle handle : m_game_scene.referenced_models()) {
            if (models_to_keep.find(handle) == models_to_keep.end()) {
                m_model_manager.free_model(handle);
            }
        }

        std::ifstream in(m_scene_manager.queued_scene_path(), std::ios::binary);
        m_game_scene.deserialize(in);
        in.close();

        m_scene_manager.reset_queue();

        on_game_scene_start();
    }
}

void Context::on_game_scene_start() {
    for (UUID uuid : m_project.autoload_scripts()) {
        NodeID id = m_game_scene.add_node_to_root(Script::get_script_name(uuid));
        m_game_scene.add_component<ScriptSet>(id);
        ScriptSet & script_set = m_game_scene.get_component<ScriptSet>(id);
        script_set.add_script_from_uuid(m_game_scene, uuid);
    }
    m_game_scene.start();
}
