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

std::unordered_map<UUID, Script::TScript>
    Script::s_constructors;

Script * Script::deserialize(
    UUID uuid,
    std::istream & in,
    Scene & scene,
    NodeID node_id
) {
    if (auto it = s_constructors.find(uuid); it != s_constructors.end()) {
        return it->second(in, scene, node_id);
    }

    return nullptr;
}

bool Script::Register(
    UUID uuid,
    Script::TScript constructor
) {
    if (s_constructors.find(uuid) != s_constructors.end()) {
        return false;
    }
    s_constructors[uuid] = constructor;
    return true;
}