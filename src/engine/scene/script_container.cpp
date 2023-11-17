#include "script_container.h"

#include <unordered_set>

using namespace prt3;

ScriptContainer & ScriptContainer::operator=(ScriptContainer const & other) {
    m_scripts.clear();

    for (auto & pair : other.m_scripts) {
        Script * copy = pair.second->copy();
        m_scripts[pair.first] = copy;
        m_init_queue.insert(copy);
        m_late_init_queue.insert(copy);
    }

    m_next_id = other.m_next_id;

    return *this;
}

void ScriptContainer::start(Scene & scene) {
    for (auto & pair : m_scripts) {
        pair.second->on_start(scene);
    }
}

void ScriptContainer::update(Scene & scene, float delta_time) {
    for (Script * script : m_init_queue) {
        script->on_init(scene);
    }
    m_init_queue.clear();

    for (Script * script : m_late_init_queue) {
        script->on_late_init(scene);
    }
    m_late_init_queue.clear();

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
    m_init_queue.clear();
    m_late_init_queue.clear();
    m_autoload_scripts.clear();
    m_uuid_to_autoload_script.clear();
}

ScriptID ScriptContainer::add_script(Script * script, bool autoload) {
    ScriptID id = m_next_id;
    m_scripts[id] = script;
    m_init_queue.insert(script);
    m_late_init_queue.insert(script);

    if (autoload) {
        m_autoload_scripts.insert(script);
        m_uuid_to_autoload_script[script->uuid()] = script;
    }

    ++m_next_id;

    return id;
}

void ScriptContainer::remove(ScriptID id) {
    Script * script = m_scripts.at(id);
    m_scripts.erase(id);
    if (m_init_queue.find(script) != m_init_queue.end()) {
        m_init_queue.erase(script);
    }
    if (m_late_init_queue.find(script) != m_late_init_queue.end()) {
        m_late_init_queue.erase(script);
    }
    if (m_autoload_scripts.find(script) ==
        m_autoload_scripts.end()) {
        delete script;
    } else {
        m_autoload_scripts.erase(script);
        m_uuid_to_autoload_script.erase(script->uuid());
    }
}
