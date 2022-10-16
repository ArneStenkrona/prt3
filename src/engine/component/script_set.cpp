#include "script_set.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

ScriptSet::ScriptSet(Scene & scene, NodeID node_id)
 : m_scene{scene},
   m_node_id{node_id} {

}

ScriptID ScriptSet::add_script_to_Scene(Script * script) {
    return m_scene.internal_add_script(script);
}

Script * ScriptSet::get_script_from_scene(ScriptID id) {
    return m_scene.internal_get_script(id);
}
