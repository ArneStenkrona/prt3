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
#include "src/util/serialization_util.h"

#include <unordered_map>
#include <iostream>
#include <cstddef>
#include <tuple>

namespace prt3 {

class ComponentManager {
public:
  template<typename ComponentType, typename... ArgTypes>
    ComponentType & add_component(
        Scene & scene,
        NodeID id,
        ArgTypes... args
    ) {
        return get_component_storage<ComponentType>().add(scene, id, args...);
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

    void serialize(
        std::ostream & out,
        Scene const & scene,
        std::unordered_map<NodeID, NodeID> const & compacted_ids
    ) const;

    void deserialize(std::istream & in, Scene & scene);

private:
     // Template meta programming utilities
    template <class T, class Tuple>
    struct Index;

    template <class T, class... Types>
    struct Index<T, std::tuple<T, Types...>> {
        static const std::size_t value = 0;
    };

    template <class T, class U, class... Types>
    struct Index<T, std::tuple<U, Types...>> {
        static const std::size_t value = 1 + Index<T, std::tuple<Types...>>::value;
    };

    template<typename ComponentType>
    struct ComponentStorage {
        typedef size_t InternalID;
        static constexpr InternalID NO_COMPONENT = -1;
        std::vector<InternalID> node_map;
        std::vector<ComponentType> components;

        template<typename... ArgTypes>
        ComponentType & add(Scene & scene, NodeID id, ArgTypes & ... args) {
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
            return components.at(node_map.at(id));
        }

        bool has_component(NodeID id) const {
            return static_cast<NodeID>(node_map.size()) > id &&
                   node_map.at(id) != NO_COMPONENT;
        }

        std::vector<ComponentType> const & get_all_components() const
        { return components; }

        void serialize(
            std::ostream & out,
            Scene const & scene,
            std::unordered_map<NodeID, NodeID> const & compacted_ids
        ) const {
            write_stream(out, components.size());
            for (ComponentType const & component : components) {
                write_stream(out, compacted_ids.at(component.node_id()));
                component.serialize(out, scene);
            }
        }

        void deserialize(
            std::istream & in,
            Scene & scene
        ) {
            size_t n_components;
            read_stream(in, n_components);
            for (size_t i = 0; i < n_components; ++i) {
                NodeID id;
                read_stream(in, id);
                add(scene, id, in);
            }
        }

        void clear() {
            node_map.clear();
            components.clear();
        }
    };

    typedef std::tuple<
        ComponentStorage<Material>,
        ComponentStorage<Mesh>,
        ComponentStorage<PointLightComponent>,
        ComponentStorage<ColliderComponent>,
        ComponentStorage<ScriptSet>
    > ComponentStorageTypes;

    ComponentStorageTypes m_component_storages;

    template<typename ComponentType>
    ComponentStorage<ComponentType> const & get_component_storage() const {
        return std::get<
            Index<ComponentStorage<ComponentType>,
                  ComponentStorageTypes>::value
            >(m_component_storages);
    }

    template<typename ComponentType>
    ComponentStorage<ComponentType> & get_component_storage() {
        return std::get<
            Index<ComponentStorage<ComponentType>,
                  ComponentStorageTypes>::value
            >(m_component_storages);
    }

    void clear();

    template<size_t I = 0, typename... Tp>
    void clear_storage(
        std::tuple<Tp...> & t
    ) {
        auto & storage = std::get<I>(t);
        storage.clear();

        if constexpr(I+1 != sizeof...(Tp))
            clear_storage<I+1>(t);
    }

    static constexpr uint64_t magic_num = 2407081819398577441ull;
    template<size_t I = 0, typename... Tp>
    void serialize_storage(
        std::ostream & out,
        Scene const & scene,
        std::unordered_map<NodeID, NodeID> const & compacted_ids,
        std::tuple<Tp...> const & t
    ) const {
        uint64_t magic_num_i = magic_num + I;
        write_stream(out, magic_num_i);

        auto const & storage = std::get<I>(t);
        storage.serialize(out, scene, compacted_ids);

        if constexpr(I+1 != sizeof...(Tp))
            serialize_storage<I+1>(out, scene, compacted_ids, t);
    }

    template<size_t I = 0, typename... Tp>
    void deserialize_storage(
        std::istream & in,
        Scene & scene,
        std::tuple<Tp...> & t
    ) {
        uint64_t magic_num_i = magic_num + I;
        uint64_t r_magic_num_i;
        read_stream(in, r_magic_num_i);
        if (r_magic_num_i != magic_num_i) {
            // TODO: error handling
            std::cout << "deserialize_storage(): error!" << std::endl;
        }

        auto & storage = std::get<I>(t);
        storage.deserialize(in, scene);

        if constexpr(I+1 != sizeof...(Tp))
            deserialize_storage<I+1>(in, scene, t);
    }

    friend class Scene;
};

} // namespace prt3

#endif
