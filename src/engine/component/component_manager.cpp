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
    write_stream(out, std::tuple_size_v<ComponentStoragesType>);
    serialize_storage(out, scene, compacted_ids, m_component_storages);
}

void ComponentManager::deserialize(std::istream & in, Scene & scene) {
    size_t n_storages;
    read_stream(in, n_storages);
    deserialize_storage(in, scene, n_storages, m_component_storages);
}

void ComponentManager::serialize_components(
    std::ostream & out,
    Scene const & scene,
    NodeID id
) const {
    write_stream(out, std::tuple_size_v<ComponentStoragesType>);
    serialize_components(out, scene, id, m_component_storages);
}

void ComponentManager::deserialize_components(
    std::istream & in,
    Scene & scene,
    NodeID id
) {
    size_t n_components;
    read_stream(in, n_components);
    deserialize_components(in, scene, id, n_components, m_component_storages);
}
