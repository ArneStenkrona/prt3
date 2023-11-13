#include "project.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"

using namespace prt3;

void Project::serialize(std::ostream & out) const {
    write_string(out, m_main_scene_path);

    write_stream(out, m_autoload_scripts.size());
    for (UUID uuid : m_autoload_scripts) {
        write_stream(out, uuid);
    }
}

Project::~Project() {
    for (Script * script : m_active_scripts) {
        delete script;
    }
}

void Project::deserialize(std::istream & in) {
    read_string(in, m_main_scene_path);

    size_t n_scripts;
    read_stream(in, n_scripts);
    for (size_t i = 0; i < n_scripts; ++i) {
        UUID uuid;
        read_stream(in, uuid);
        m_autoload_scripts.insert(uuid);
    }
}

void Project::on_game_start(Scene & initial_scene) {
    m_active_scripts.resize(m_autoload_scripts.size());
    unsigned i = 0;
    for (UUID uuid : m_autoload_scripts) {
        Script * script = Script::instantiate(
            uuid,
            initial_scene,
            NO_NODE
        );

        m_active_scripts[i] = script;
        ++i;

        initial_scene.internal_add_script(script, true);
    }
}

void Project::on_game_end(Scene & scene) {
    for (Script * script : m_active_scripts) {
        script->on_game_end(scene);
        delete script;
    }
    m_active_scripts.clear();
}
