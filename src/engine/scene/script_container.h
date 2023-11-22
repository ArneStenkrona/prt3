#ifndef PRT3_SCRIPT_CONTAINER_H
#define PRT3_SCRIPT_CONTAINER_H

#include "src/engine/component/script/script.h"

#include <unordered_map>
#include <unordered_set>

namespace prt3 {

class ScriptSet;

class ScriptContainer {
public:
    ScriptContainer & operator=(ScriptContainer const & other);

    void start(Scene & scene);
    void update(Scene & scene, float delta_time);
    void clear();

    ScriptID add_script(Scene & scene, Script * script, bool autoload);

    void remove(ScriptID id);

    std::unordered_map<ScriptID, Script *> const & scripts() const
    { return m_scripts; }

    std::unordered_map<ScriptID, Script *> & scripts() { return m_scripts; }

    Script * get_script(ScriptID id) { return m_scripts.at(id); }

    Script * get_autoload_script(UUID uuid)
    { return m_uuid_to_autoload_script.at(uuid); }

private:
    std::unordered_map<ScriptID, Script *> m_scripts;
    std::unordered_set<Script *> m_autoload_scripts;
    std::unordered_set<Script *> m_unitialized;
    std::unordered_map<UUID, Script *> m_uuid_to_autoload_script;

    ScriptID m_next_id = 0;

    ScriptID add_script(
        Scene & scene,
        Script * script,
        bool autoload,
        bool init
    );

    void init_unitialized(Scene & scene);

    friend class ScriptSet;
    friend class Prefab;
};

} // namespace prt3

#endif // SCRIPT_CONTAINER_H
