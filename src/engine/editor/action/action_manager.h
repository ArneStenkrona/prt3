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
            ++m_action_count;

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

    void clear() {
        clear_history();
        clear_future();
    }

    size_t get_action_count() const { return m_action_count; }
    void reset_action_count() { m_action_count = 0; }

private:
    EditorContext & m_editor_context;

    static constexpr size_t MAX_HISTORY_SIZE = 512;
    RingBuffer<Action*, MAX_HISTORY_SIZE> m_history;
    std::vector<Action*> m_future;

    size_t m_action_count = 0;

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
