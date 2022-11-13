#ifndef PRT3_SCRIPT_SET_H
#define PRT3_SCRIPT_SET_H

#include "src/engine/component/script/script.h"

#include <vector>

namespace prt3 {

class Scene;

template<typename T>
class ComponentStorage;

class EditorContext;
template<typename T>
void inner_show_component(EditorContext &, NodeID);

class ScriptSet {
public:
    ScriptSet(Scene & scene, NodeID node_id);
    ScriptSet(Scene & scene, NodeID node_id, std::istream & in);

    template<class T>
    ScriptID add_script(Scene & scene) {
        ScriptID existing = get_script_id<T>(scene);
        if (existing != NO_SCRIPT) { return existing; }

        T * script = new T(scene, m_node_id);
        ScriptID id = add_script_to_scene(scene, script);
        m_script_ids.push_back(id);
        return id;
    }

    template<typename T>
    T * get_script(Scene & scene) {
        for (ScriptID const & id : m_script_ids) {
            Script * script = get_script_from_scene(scene, id);
            T * t_script = dynamic_cast<T *>(script);
            if (t_script) {
                return t_script;
            }
        }
        return nullptr;
    }

    template<typename T>
    ScriptID get_script_id(Scene & scene) {
        for (ScriptID const & id : m_script_ids) {
            Script * script = get_script_from_scene(scene, id);
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

    NodeID node_id() const { return m_node_id; }

    bool remove_script(Scene & scene, ScriptID id);

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Script Set"; }

private:
    std::vector<ScriptID> m_script_ids;
    NodeID m_node_id;

    ScriptID add_script_to_scene(Scene & scene, Script * script);
    Script * get_script_from_scene(Scene & scene, ScriptID id);

    void add_script_from_uuid(Scene & scene, UUID uuid);

    void remove(Scene & scene);

    friend class ComponentStorage<ScriptSet>;
    friend void inner_show_component<ScriptSet>(EditorContext &, NodeID);
};

} // namespace prt3

#endif
