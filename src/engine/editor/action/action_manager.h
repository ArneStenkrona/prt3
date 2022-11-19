#ifndef PRT3_ACTION_MANAGER_H
#define PRT3_ACTION_MANAGER_H

#include "src/engine/editor/action/action.h"
#include "src/util/ring_buffer.h"

#include <vector>

namespace prt3 {

class EditorContext;

class ActionManager {
public:
    ActionManager(EditorContext & editor_context);
    ~ActionManager();

    template<typename ActionType, typename... ArgTypes>
    bool perform(ArgTypes... args) {
        Action * action = new ActionType(m_editor_context, args...);
        if (action->apply()) {
            clear_future();
            add_to_history(action);
            return true;
        } else {
            delete action;
            return false;
        }
    }

    bool undo();
    bool redo();

private:
    EditorContext & m_editor_context;

    static constexpr size_t MAX_HISTORY_SIZE = 100;
    RingBuffer<Action*, MAX_HISTORY_SIZE> m_history;
    std::vector<Action*> m_future;

    void add_to_history(Action * action) {
        if (m_history.full()) {
            delete m_history.front();
        }

        m_history.push_back(action);
    }

    void clear_history();
    void clear_future();
};

} // namespace prt3

#endif
