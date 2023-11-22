#include "script_container.h"

#include "src/engine/scene/scene.h"

#include <unordered_set>

using namespace prt3;

ScriptContainer & ScriptContainer::operator=(ScriptContainer const & other) {
    m_scripts.clear();

    for (auto & pair : other.m_scripts) {
        Script * copy = pair.second->copy();
        m_scripts[pair.first] = copy;
    }

    m_next_id = other.m_next_id;

    return *this;
}

ScriptID ScriptContainer::add_script(
    Scene & scene,
    Script * script,
    bool autoload
) {
    return add_script(scene, script, autoload, scene.game_is_active());
}

void ScriptContainer::start(Scene & scene) {
    for (auto & pair : m_scripts) {
        if (m_autoload_scripts.find(pair.second) != m_autoload_scripts.end()) {
            // on_init has already been called when we added the autoload script
            continue;
        }

        pair.second->on_init(scene);
    }

    for (auto & pair : m_scripts) {
        pair.second->on_start(scene);
    }

    for (auto & pair : m_scripts) {
        pair.second->on_late_start(scene);
    }
}

void ScriptContainer::update(Scene & scene, float delta_time) {
    for (auto & pair : m_scripts) {
        pair.second->on_update(scene, delta_time);
    }

    for (auto & pair : m_scripts) {
        pair.second->on_late_update(scene, delta_time);
    }
}

void ScriptContainer::clear() {
    for (auto & pair : m_scripts) {
        if (m_autoload_scripts.find(pair.second) ==
            m_autoload_scripts.end()) {
            delete pair.second;
        }
    }

    m_scripts.clear();
    m_autoload_scripts.clear();
    m_uuid_to_autoload_script.clear();
}

ScriptID ScriptContainer::add_script(
    Scene & scene,
    Script * script,
    bool autoload,
    bool init
) {
    ScriptID id = m_next_id;
    m_scripts[id] = script;

    if (autoload) {
        m_autoload_scripts.insert(script);
        m_uuid_to_autoload_script[script->uuid()] = script;
    }

    if (init) {
        script->on_init(scene);
    } else {
        m_unitialized.insert(script);
    }

    ++m_next_id;

    return id;
}

void ScriptContainer::remove(ScriptID id) {
    Script * script = m_scripts.at(id);
    m_scripts.erase(id);

    if (m_autoload_scripts.find(script) ==
        m_autoload_scripts.end()) {
        delete script;
    } else {
        m_autoload_scripts.erase(script);
        m_uuid_to_autoload_script.erase(script->uuid());
    }
}

void ScriptContainer::init_unitialized(Scene & scene) {
    for (Script * script : m_unitialized) {
        script->on_init(scene);
    }
    m_unitialized.clear();
}
