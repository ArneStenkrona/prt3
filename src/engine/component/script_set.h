#ifndef PRT3_SCRIPT_SET_H
#define PRT3_SCRIPT_SET_H

#include "src/engine/component/script/script.h"

#include <vector>

namespace prt3 {

class Scene;

class ScriptSet {
public:
    ScriptSet(Scene & scene, NodeID node_id);
    ScriptSet(Scene & scene, NodeID node_id, std::istream & in);

    template<class T>
    ScriptID add_script() {
        ScriptID existing = get_script_id<T>();
        if (existing != NO_SCRIPT) { return existing; }

        T * script = new T(m_scene, m_node_id);
        ScriptID id = add_script_to_Scene(script);
        m_script_ids.push_back(id);
        return id;
    }

    template<typename T>
    T * get_script() {
        for (ScriptID const & id : m_script_ids) {
            Script * script = get_script_from_scene(id);
            T * t_script = dynamic_cast<T *>(script);
            if (t_script) {
                return t_script;
            }
        }
        return nullptr;
    }

    template<typename T>
    ScriptID get_script_id() {
        for (ScriptID const & id : m_script_ids) {
            Script * script = get_script_from_scene(id);
            T * t_script = dynamic_cast<T *>(script);
            if (t_script) {
                return id;
            }
        }
        return NO_SCRIPT;
    }

    std::vector<ScriptID> const & get_all_scripts() const {
        return m_script_ids;
    }

    Scene const & scene() const { return m_scene; }
    Scene & scene() { return m_scene; }

    NodeID node_id() const { return m_node_id; }

private:
    std::vector<ScriptID> m_script_ids;
    Scene & m_scene;
    NodeID m_node_id;

    ScriptID add_script_to_Scene(Script * script);
    Script * get_script_from_scene(ScriptID id);
};

std::ostream & operator << (
    std::ostream & out,
    ScriptSet const & component
);

} // namespace prt3

#endif
