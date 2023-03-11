#include "context.h"

#include "src/engine/component/script/script.h"
#include "src/engine/component/script_set.h"
#include "src/util/mem.h"

#include <iostream>
#include <fstream>
#include <sstream>
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

void Context::load_scene_if_queued() {
    if (m_scene_manager.scene_queued()) {
        auto start_time = std::chrono::high_resolution_clock::now();

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

        /* save autoload state */
        std::stringstream out_autoload;
        thread_local std::vector<char> data;

        for (UUID uuid : m_project.autoload_scripts()) {
            Script const * script = m_game_scene.get_autoload_script(uuid);
            if (script != nullptr) {
                write_stream(out_autoload, true);
                script->save_state(m_game_scene, out_autoload);
            } else {
                write_stream(out_autoload, false);
            }
        }

        std::string const & s = out_autoload.str();
        data.reserve(s.size());
        data.assign(s.begin(), s.end());

        /* load scene */
        std::ifstream in(m_scene_manager.queued_scene_path(), std::ios::binary);
        m_game_scene.deserialize(in);
        in.close();

        m_scene_manager.reset_queue();
        m_game_scene.add_autoload_scripts(m_project.autoload_scripts());

        /* restore autoload state */
        imemstream in_autoload(data.data(), data.size());

        for (UUID uuid : m_project.autoload_scripts()) {
            bool serialized;
            read_stream(in_autoload, serialized);

            if (serialized) {
                Script * script = m_game_scene.get_autoload_script(uuid);
                script->restore_state(m_game_scene, in_autoload);
            }
        }

        /* start scene */
        m_game_scene.start();

        auto end_time = std::chrono::high_resolution_clock::now();

        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>
            (end_time-start_time);

        std::cout << "load time: " << duration.count() << " ms" << std::endl;
    }
}
