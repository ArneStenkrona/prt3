#include "component_manager.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

void ComponentManager::clear() {
    clear_storage(m_component_storages);
}

void ComponentManager::serialize(
    std::ostream & out,
    Scene const & scene,
    std::unordered_map<NodeID, NodeID> const & compacted_ids
) const {
    serialize_storage(out, scene, compacted_ids, m_component_storages);
}

void ComponentManager::deserialize(std::istream & in, Scene & scene) {
    deserialize_storage(in, scene, m_component_storages);
}

void ComponentManager::serialize_components(
    std::ostream & out,
    Scene const & scene,
    NodeID id
) const {
    serialize_components(out, scene, id, m_component_storages);
}

void ComponentManager::deserialize_components(
    std::istream & in,
    Scene & scene,
    NodeID id
) {
    deserialize_components(in, scene, id, m_component_storages);
}
