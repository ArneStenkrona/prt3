#include "script_set.h"

#include "src/engine/scene/scene.h"

#include "src/util/serialization_util.h"

using namespace prt3;

ScriptSet::ScriptSet(Scene & scene, NodeID node_id)
 : m_scene{scene},
   m_node_id{node_id} {
}

ScriptSet::ScriptSet(
    Scene & scene,
    NodeID node_id,
    std::istream & in
)
 : m_scene{scene},
   m_node_id{node_id} {
    size_t n_scripts;
    read_stream(in, n_scripts);

    for (size_t i = 0; i < n_scripts; ++i) {
        UUID uuid;
        read_stream(in, uuid);

        Script * script = Script::deserialize(
            uuid,
            in,
            m_scene,
            node_id
        );

        ScriptID id = add_script_to_Scene(script);
        m_script_ids.push_back(id);
    }
}

ScriptID ScriptSet::add_script_to_Scene(Script * script) {
    return m_scene.internal_add_script(script);
}

Script * ScriptSet::get_script_from_scene(ScriptID id) {
    return m_scene.internal_get_script(id);
}

namespace prt3 {

std::ostream & operator << (
    std::ostream & out,
    ScriptSet const & component
) {
    write_stream(out, component.get_all_scripts().size());

    Scene const & scene = component.scene();
    for (ScriptID id : component.get_all_scripts()) {
        Script const * script = scene.get_script<Script>(id);
        script->serialize(out);
    }
    return out;
}

} // namespace prt3
