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
        Script * script = Script::deserialize(
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

Script const * ScriptSet::get_script_from_scene(
    Scene const & scene,
    ScriptID id
) const {
    return scene.internal_get_script(id);
}

bool ScriptSet::remove_script(Scene & scene, ScriptID id) {
    for (auto it = m_script_ids.begin();
         it != m_script_ids.end();
         ++it) {
        if (*it == id) {
            scene.internal_remove_script(id);
            m_script_ids.erase(it);
            return true;
        }
    }
    return false;
}

bool ScriptSet::remove_script(Scene & scene, UUID uuid) {
    for (auto it = m_script_ids.begin();
         it != m_script_ids.end();
         ++it) {
        if (scene.internal_get_script(*it)->uuid() == uuid) {
            scene.internal_remove_script(*it);
            m_script_ids.erase(it);
            return true;
        }
    }
    return false;
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

ScriptID ScriptSet::add_script_from_uuid(Scene & scene, UUID uuid) {
    Script * script = Script::instantiate(
        uuid,
        scene,
        m_node_id
    );

    ScriptID id = add_script_to_scene(scene, script);
    m_script_ids.push_back(id);

    return id;
}
