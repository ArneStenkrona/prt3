#ifndef PRT3_COMPONENT_H
#define PRT3_COMPONENT_H

#include "src/engine/scene/node.h"
#include "src/engine/component/canvas/canvas.h"
#include "src/engine/component/animated_mesh.h"
#include "src/engine/component/animated_model.h"
#include "src/engine/component/armature.h"
#include "src/engine/component/collider_component.h"
#include "src/engine/component/decal.h"
#include "src/engine/component/mesh.h"
#include "src/engine/component/door.h"
#include "src/engine/component/material.h"
#include "src/engine/component/model.h"
#include "src/engine/component/navigation_mesh.h"
#include "src/engine/component/particle_system.h"
#include "src/engine/component/point_light.h"
#include "src/engine/component/script_set.h"
#include "src/engine/component/sound_source.h"
#include "src/engine/component/weapon.h"
#include "src/util/template_util.h"
#include "src/util/log.h"

#include <vector>
#include <unordered_map>

namespace prt3 {

class Scene;

template<typename ComponentType>
class ComponentStorage {
public:
    template<typename... ArgTypes>
    ComponentType & add(Scene & scene, NodeID id, ArgTypes && ... args) {
        if (static_cast<NodeID>(node_map.size()) <= id) {
            node_map.resize(id + 1, NO_COMPONENT);
        }

        if (!has_component(id)) {
            node_map[id] = components.size();
            components.emplace_back(scene, id, args...);
        }
        // TODO: handle existing component

        return get(id);
    }

    ComponentType const & get(NodeID id) const {
        return components[node_map[id]];
    }

    ComponentType & get(NodeID id) {
        return components[node_map[id]];
    }

    bool has_component(NodeID id) const {
        return static_cast<NodeID>(node_map.size()) > id &&
               node_map.at(id) != NO_COMPONENT;
    }

    std::vector<ComponentType> & get_all_components()
    { return components; }

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

    bool remove_component(Scene & scene, NodeID id) {
        if (!has_component(id)) {
            return false;
        }

        InternalID c_id = node_map[id];
        components[c_id].remove(scene);
        node_map[id] = NO_COMPONENT;

        if (c_id + 1 != components.size()) {
            NodeID swap = components.back().node_id();
            components[c_id] = components.back();
            node_map[swap] = c_id;
        }

        components.pop_back();

        return true;
    }

private:
    typedef size_t InternalID;
    static constexpr InternalID NO_COMPONENT = -1;
    std::vector<InternalID> node_map;
    std::vector<ComponentType> components;
};

namespace {

template<template<typename...> class T, typename>
struct wrap_arg_pack_in_storage { };

template<template<typename...> class T, typename... Ts>
struct wrap_arg_pack_in_storage<T, type_pack<Ts...> > {
    using type = T<ComponentStorage<Ts>...>;
};

}

using ComponentTypes = type_pack<
    MaterialComponent,
    Mesh,
    ModelComponent,
    PointLightComponent,
    ColliderComponent,
    ScriptSet,
    AnimatedModel,
    AnimatedMesh,
    Armature,
    Door,
    NavigationMeshComponent,
    SoundSourceComponent,
    Weapon,
    Decal,
    Canvas,
    ParticleSystem
>;

using ComponentStoragesType = wrap_arg_pack_in_storage<std::tuple, ComponentTypes>::type;

} // namespace prt3

#endif
