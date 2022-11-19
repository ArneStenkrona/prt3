#include "action_manager.h"

using namespace prt3;

ActionManager::ActionManager(
    EditorContext & editor_context
) : m_editor_context{editor_context}
{}

ActionManager::~ActionManager() {
    clear_history();
    clear_future();
}

bool ActionManager::undo() {

    if (m_history.empty()) {
        return false;
    }

    Action * action = m_history.back();
    if (action->unapply()) {
        m_future.push_back(action);
        m_history.pop_back();
        return true;
    } else {
        clear_history();
    }
    return false;
}

bool ActionManager::redo() {

    if (m_future.empty()) {
        return false;
    }

    Action * action = m_future.back();
    if (action->apply()) {
        add_to_history(action);
        m_future.pop_back();
        return true;
    } else {
        clear_future();
    }

    return false;
}

void ActionManager::clear_history() {
    while (!m_history.empty()) {
        delete m_history.back();
        m_history.pop_back();
    }
}

void ActionManager::clear_future() {
    for (Action * action : m_future) {
        delete action;
    }
    m_future.clear();
}
