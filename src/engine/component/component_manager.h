#ifndef PRT3_COMPONENT_MANAGER_H
#define PRT3_COMPONENT_MANAGER_H

#include "src/engine/scene/node.h"
#include "src/engine/physics/physics_system.h"
#include "src/engine/rendering/light.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/component/collider_component.h"
#include "src/engine/component/mesh.h"
#include "src/engine/component/material.h"
#include "src/engine/component/point_light.h"
#include "src/engine/component/script_set.h"

#include <unordered_map>
#include <iostream>

namespace prt3 {

class ComponentManager {
public:
    ComponentManager(Scene & scene);

    template<class... Types>
    void f(Types... args);

    template<typename ComponentType, typename... ArgTypes>
    ComponentType & add_component(NodeID id, ArgTypes... args) {
        return get_component_storage<ComponentType>().add(m_scene, id, args...);
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
    ComponentType const & get_component(NodeID id) const {
        return get_component_storage<ComponentType>().get(id);
    }

    template<typename ComponentType>
    std::vector<ComponentType> const & get_all_components() const {
        return get_component_storage<ComponentType>().get_all_components();
    }

    template<typename ComponentType>
    bool has_component(NodeID id) const {
        return get_component_storage<ComponentType>().has_component(id);
    }

private:
    template<typename ComponentType>
    struct ComponentStorage {
        typedef size_t InternalID;
        static constexpr InternalID NO_COMPONENT = -1;
        std::vector<InternalID> node_map;
        std::vector<ComponentType> components;

        template<typename... ArgTypes>
        ComponentType & add(Scene & scene, NodeID id, ArgTypes... args) {
            if (static_cast<NodeID>(node_map.size()) <= id) {
                node_map.resize(id + 1);
                node_map[id] = components.size();
                components.emplace_back(scene, id, args...);
                return components.back();
            } else {
                return get(id);
            }
        }

        ComponentType & get(NodeID id) {
            return components[node_map[id]];
        }

        ComponentType const & get(NodeID id) const {
            return components[node_map[id]];
        }

        bool has_component(NodeID id) const {
            return static_cast<NodeID>(node_map.size()) > id &&
                   node_map[id] != NO_COMPONENT;
        }

        std::vector<ComponentType> const & get_all_components() const
        { return components; }
    };

    ComponentStorage<Mesh> m_meshes;
    ComponentStorage<Material> m_materials;
    ComponentStorage<PointLightComponent> m_point_lights;
    ComponentStorage<ColliderComponent> m_colliders;
    ComponentStorage<ScriptSet> m_script_sets;

    template<typename ComponentType>
    ComponentStorage<ComponentType> & get_component_storage() {
        return const_cast<ComponentStorage<ComponentType>&>(
            const_cast<const ComponentManager*>(this)
                ->get_component_storage<ComponentType>()
        );
    }

    template<typename ComponentType>
    ComponentStorage<ComponentType> const & get_component_storage() const;

    template<>
    ComponentStorage<Mesh> const & get_component_storage() const
    { return m_meshes; }

    template<>
    ComponentStorage<Material> const & get_component_storage() const
    { return m_materials; }

    template<>
    ComponentStorage<PointLightComponent> const & get_component_storage() const
    { return m_point_lights; }

    template<>
    ComponentStorage<ColliderComponent> const & get_component_storage() const
    { return m_colliders; }

    template<>
    ComponentStorage<ScriptSet> const & get_component_storage() const
    { return m_script_sets; }

    Scene & m_scene;
};

} // namespace prt3

#endif
