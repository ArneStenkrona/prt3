#include "context.h"

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

void Context::load_scene_if_queued() {
    if (m_scene_manager.scene_queued()) {
        static std::unordered_set<ModelHandle> models_to_keep;
        static std::unordered_set<ModelHandle> models_to_delete;

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

        // TODO: clean up now unreferenced gpu resources

        m_scene_manager.reset_queue();
    }
}
