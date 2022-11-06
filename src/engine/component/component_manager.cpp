#include "component_manager.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

ComponentManager::ComponentManager(Scene & scene)
 : m_scene{scene} {
}

void ComponentManager::clear() {
    clear_storage(m_component_storages);
}

void ComponentManager::serialize(
    std::ostream & out,
    std::unordered_map<NodeID, NodeID> const & compacted_ids
) const {
    serialize_storage(out, compacted_ids, m_component_storages);
}

void ComponentManager::deserialize(std::istream & in) {
    deserialize_storage(in, m_component_storages);
}
