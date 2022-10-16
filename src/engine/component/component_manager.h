#ifndef PRT3_COMPONENT_MANAGER_H
#define PRT3_COMPONENT_MANAGER_H

#include "src/engine/scene/node.h"
#include "src/engine/physics/physics_system.h"
#include "src/engine/rendering/light.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/component/script_set.h"

#include <unordered_map>
#include <iostream>

namespace prt3 {

class ComponentManager {
public:
    ComponentManager(Scene & scene);

    template<typename ComponentType>
    ComponentType & add_component(NodeID id) {
        return get_component_storage<ComponentType>().add(m_scene, id);
    }

    /**
     * Note: reference should be considered stale if
     *       any components of same type is added
     *       or removed
     */
    template<typename ComponentType>
    ComponentType & get_component(NodeID id) {
        return get_component_storage<ComponentType>().get(id);
    }

    template<typename ComponentType>
    bool has_component(NodeID id) {
        return get_component_storage<ComponentType>().has_component(id);
    }
     /* TODO: refactor legacy component system */
    void set_mesh_component(NodeID node, ResourceID mesh)
        { m_mesh_components[node] = mesh; }
    void set_material_component(NodeID node, ResourceID material)
        { m_material_components[node] = material; }
    void set_point_light_component(NodeID node, PointLight const & light)
        { m_point_light_components[node] = light; }

    std::unordered_map<NodeID, ResourceID> const & get_mesh_components() const
        { return m_mesh_components; }
    std::unordered_map<NodeID, ResourceID> const & get_material_components() const
        { return m_material_components; }
    std::unordered_map<NodeID, PointLight> const & get_point_light_components() const
        { return m_point_light_components; }

private:
    std::unordered_map<NodeID, ResourceID> m_mesh_components;
    std::unordered_map<NodeID, ResourceID> m_material_components;
    std::unordered_map<NodeID, PointLight> m_point_light_components;

    template<typename ComponentType>
    struct ComponentStorage {
        typedef size_t InternalID;
        static constexpr InternalID NO_COMPONENT = -1;
        std::vector<InternalID> node_map;
        std::vector<ComponentType> components;

        ComponentType & add(Scene & scene, NodeID id) {
            if (static_cast<NodeID>(node_map.size()) <= id) {
                node_map.resize(id + 1);
                node_map[id] = components.size();
                components.emplace_back(scene, id);
                return components.back();
            } else {
                return get(id);
            }
        }

        ComponentType & get(NodeID id) {
            return components[node_map[id]];
        }

        bool has_component(NodeID id) {
            return static_cast<NodeID>(node_map.size()) > id &&
                   node_map[id] != NO_COMPONENT;
        }
    };

    ComponentStorage<ScriptSet> m_script_sets;

    template<typename ComponentType>
    ComponentStorage<ComponentType> & get_component_storage();

    template<>
    ComponentStorage<ScriptSet> & get_component_storage()
        { return m_script_sets; }

    Scene & m_scene;
};

} // namespace prt3

#endif
