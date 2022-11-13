#include "script.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

Script::Script(Scene &, NodeID node_id)
 : m_node_id{node_id} {
}

Node & Script::get_node(Scene & scene) { return scene.get_node(m_node_id); }

bool Script::add_tag(Scene & scene, NodeTag const & tag) {
    return scene.add_tag_to_node(tag, m_node_id);
}

std::unordered_map<UUID, Script::TScriptDeserializer>
    Script::s_deserializers;

std::unordered_map<UUID, Script::TScriptInstantiator>
    Script::s_instantiators;


std::unordered_map<UUID, char const *>
    Script::s_script_names;

Script * Script::deserialize(
    UUID uuid,
    std::istream & in,
    Scene & scene,
    NodeID node_id
) {
    if (auto it = s_deserializers.find(uuid); it != s_deserializers.end()) {
        return it->second(in, scene, node_id);
    }

    return nullptr;
}

Script * Script::instantiate(
    UUID uuid,
    Scene & scene,
    NodeID node_id
) {
    if (auto it = s_instantiators.find(uuid); it != s_instantiators.end()) {
        return it->second(scene, node_id);
    }

    return nullptr;
}

bool Script::Register(
    UUID uuid,
    char const * name,
    Script::TScriptDeserializer deserializer,
    Script::TScriptInstantiator instantiator
) {
    if (s_deserializers.find(uuid) != s_deserializers.end()) {
        return false;
    }
    s_script_names[uuid] = name;
    s_deserializers[uuid] = deserializer;
    s_instantiators[uuid] = instantiator;
    return true;
}
