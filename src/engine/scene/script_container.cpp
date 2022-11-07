#include "script_container.h"

#include <unordered_set>

using namespace prt3;

ScriptContainer::~ScriptContainer() {
    for (Script * script : m_scripts) {
        delete script;
    }
}

ScriptContainer & ScriptContainer::operator=(ScriptContainer const & other) {
    for (Script * script : m_scripts) {
        delete script;
    }

    m_scripts = other.m_scripts;

    m_scripts.resize(other.m_scripts.size());

    for (size_t i = 0; i < m_scripts.size(); ++i) {
        m_scripts[i] = other.m_scripts[i]->copy();
    }

    m_init_queue = m_scripts;
    m_late_init_queue = m_scripts;

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

    for (Script * script : m_scripts) {
        script->on_update(scene, delta_time);
    }

    for (Script * script : m_scripts) {
        script->on_late_update(scene, delta_time);
    }
}

void ScriptContainer::clear() {
    m_scripts.clear();
    m_init_queue.clear();
    m_late_init_queue.clear();
}
