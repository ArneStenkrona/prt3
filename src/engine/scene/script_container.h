#ifndef PRT3_SCRIPT_CONTAINER_H
#define PRT3_SCRIPT_CONTAINER_H

#include "src/engine/component/script/script.h"

#include <unordered_map>
#include <unordered_set>

namespace prt3 {

class ScriptContainer {
public:
    ScriptContainer & operator=(ScriptContainer const & other);

    void start(Scene & scene);
    void update(Scene & scene, float delta_time);
    void clear();

    ScriptID add_script(Script * script);

    void remove(ScriptID id);

    std::unordered_map<ScriptID, Script *> const & scripts() const
    { return m_scripts; }

    std::unordered_map<ScriptID, Script *> & scripts() { return m_scripts; }

    Script * get_script(ScriptID id) { return m_scripts.at(id); }

private:
    std::unordered_map<ScriptID, Script *> m_scripts;
    std::unordered_set<Script *> m_init_queue;
    std::unordered_set<Script *> m_late_init_queue;

    ScriptID m_next_id = 0;
};

} // namespace prt3

#endif
