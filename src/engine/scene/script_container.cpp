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

    return *this;
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
        delete pair.second;
    }

    m_scripts.clear();
    m_init_queue.clear();
    m_late_init_queue.clear();
}

ScriptID ScriptContainer::add_script(Script * script) {
    ScriptID id = m_next_id;
    m_scripts[id] = script;
    m_init_queue.insert(script);
    m_late_init_queue.insert(script);

    ++m_next_id;

    return id;
}

void ScriptContainer::remove(ScriptID id) {
    Script * script = m_scripts[id];
    m_scripts.erase(id);
    if (m_init_queue.find(script) != m_init_queue.end()) {
        m_init_queue.erase(script);
    }
    if (m_late_init_queue.find(script) != m_late_init_queue.end()) {
        m_late_init_queue.erase(script);
    }
}
