#include "script_set.h"

#include "src/engine/scene/scene.h"

#include "src/util/serialization_util.h"

using namespace prt3;

ScriptSet::ScriptSet(Scene &, NodeID node_id)
 : m_node_id{node_id} {
}

ScriptSet::ScriptSet(
    Scene & scene,
    NodeID node_id,
    std::istream & in
)
 : m_node_id{node_id} {
    size_t n_scripts;
    read_stream(in, n_scripts);

    for (size_t i = 0; i < n_scripts; ++i) {
        UUID uuid;
        read_stream(in, uuid);

        Script * script = Script::deserialize(
            uuid,
            in,
            scene,
            node_id
        );

        ScriptID id = add_script_to_scene(scene, script);
        m_script_ids.push_back(id);
    }
}

ScriptID ScriptSet::add_script_to_scene(Scene & scene, Script * script) {
    return scene.internal_add_script(script);
}

Script * ScriptSet::get_script_from_scene(Scene & scene, ScriptID id) {
    return scene.internal_get_script(id);
}

void ScriptSet::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    write_stream(out, get_all_scripts().size());

    for (ScriptID id : get_all_scripts()) {
        Script const * script = scene.get_script<Script>(id);
        script->serialize(out);
    }
}

void ScriptSet::remove(Scene & scene) {
    for (ScriptID id : get_all_scripts()) {
        scene.remove_script(id);
    }
}
