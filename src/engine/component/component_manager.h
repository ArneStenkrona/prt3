#ifndef PRT3_COMPONENT_MANAGER_H
#define PRT3_COMPONENT_MANAGER_H

#include "src/engine/scene/node.h"
#include "src/util/serialization_util.h"
#include "src/engine/component/component.h"
#include "src/util/template_util.h"
#include "src/util/log.h"

#include <unordered_map>

namespace prt3 {

class ComponentManager {
public:
  template<typename ComponentType, typename... ArgTypes>
    ComponentType & add_component(
        Scene & scene,
        NodeID id,
        ArgTypes && ... args
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
    std::vector<ComponentType> & get_all_components() {
        return get_component_storage<ComponentType>().get_all_components();
    }

    template<typename ComponentType>
    std::vector<ComponentType> const & get_all_components() const {
        return get_component_storage<ComponentType>().get_all_components();
    }

    template<typename ComponentType>
    bool has_component(NodeID id) const {
        return get_component_storage<ComponentType>().has_component(id);
    }

    template<typename ComponentType>
    bool remove_component(Scene & scene, NodeID id) {
        return get_component_storage<ComponentType>()
            .remove_component(scene, id);
    }

    void remove_all_components(Scene & scene, NodeID id)
    { remove_components(scene, id, m_component_storages); }

    void serialize(
        std::ostream & out,
        Scene const & scene,
        std::unordered_map<NodeID, NodeID> const & compacted_ids
    ) const;

    void deserialize(std::istream & in, Scene & scene);

    void serialize_components(
        std::ostream & out,
        Scene const & scene,
        NodeID id
    ) const;

    void deserialize_components(
        std::istream & in,
        Scene & scene,
        NodeID id
    );

    template<typename ComponentType>
    void serialize_component(
        std::ostream & out,
        Scene const & scene,
        NodeID id
    ) const {
        get_component_storage<ComponentType>().get(id).serialize(out, scene);
    }

    template<typename ComponentType>
    void deserialize_component(
        std::istream & in,
        Scene & scene,
        NodeID id
    ) {
        get_component_storage<ComponentType>().add(scene, id, in);
    }

private:
    ComponentStoragesType m_component_storages;

    template<typename ComponentType>
    ComponentStorage<ComponentType> const & get_component_storage() const {
        return std::get<
            Index<ComponentStorage<ComponentType>,
                  ComponentStoragesType>::value
            >(m_component_storages);
    }

    template<typename ComponentType>
    ComponentStorage<ComponentType> & get_component_storage() {
        return std::get<
            Index<ComponentStorage<ComponentType>,
                  ComponentStoragesType>::value
            >(m_component_storages);
    }

    void update(Scene & scene) { inner_update(scene, m_component_storages); }

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
        std::tuple<ComponentStorage<Tp>...> const & t
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
        size_t n_storages,
        std::tuple<ComponentStorage<Tp>...> & t
    ) {
        if (n_storages <= I) {
            return;
        }

        uint64_t magic_num_i = magic_num + I;
        uint64_t r_magic_num_i;
        read_stream(in, r_magic_num_i);
        if (r_magic_num_i != magic_num_i) {
            // TODO: error handling
            PRT3ERROR("deserialize_store(): error!\n");
        }

        auto & storage = std::get<I>(t);
        storage.deserialize(in, scene);

        if constexpr(I+1 != sizeof...(Tp))
            deserialize_storage<I+1>(in, scene, n_storages, t);
    }

    template<size_t I = 0, typename... Tp>
    void serialize_components(
        std::ostream & out,
        Scene const & scene,
        NodeID id,
        std::tuple<ComponentStorage<Tp>...> const & t
    ) const {
        uint64_t magic_num_i = magic_num + I;
        write_stream(out, magic_num_i);

        auto const & storage = std::get<I>(t);
        if (storage.has_component(id)) {
            write_stream(out, (unsigned char)(1));
            storage.get(id).serialize(out, scene);
        } else {
            write_stream(out, (unsigned char)(0));
        }

        if constexpr(I+1 != sizeof...(Tp))
            serialize_components<I+1>(out, scene, id, t);
    }

    template<size_t I = 0, typename... Tp>
    void deserialize_components(
        std::istream & in,
        Scene & scene,
        NodeID id,
        size_t n_components,
        std::tuple<ComponentStorage<Tp>...> & t
    ) {
        if (n_components <= I) {
            return;
        }

        uint64_t magic_num_i = magic_num + I;
        uint64_t r_magic_num_i;
        read_stream(in, r_magic_num_i);
        if (r_magic_num_i != magic_num_i) {
            // TODO: error handling
            PRT3ERROR("deserialize_components(): error!\n");
        }

        unsigned char has;
        read_stream(in, has);

        if (has == 1) {
            auto & storage = std::get<I>(t);
            storage.add(scene, id, in);
        }

        if constexpr(I+1 != sizeof...(Tp))
            deserialize_components<I+1>(in, scene, id, n_components, t);
    }

    template<size_t I = 0, typename... Tp>
    void remove_components(
        Scene & scene,
        NodeID id,
        std::tuple<ComponentStorage<Tp>...> & t
    ) {
        auto & storage = std::get<I>(t);
        storage.remove_component(scene, id);

        if constexpr(I+1 != sizeof...(Tp))
            remove_components<I+1>(scene, id, t);
    }

    template <typename T, typename = int>
    struct HasUpdate : std::false_type {
    };

    template <typename T>
    struct HasUpdate<T, decltype(&T::update, 0)> : std::true_type {
    };

    template<typename T>
    inline constexpr void update_if_exists(
        Scene & scene,
        ComponentStorage<T> & storage
    ) {
        if constexpr (HasUpdate<T>::value)
            T::update(scene, storage.get_all_components());
    }

    template<size_t I = 0, typename... Tp>
    void inner_update(
        Scene & scene,
        std::tuple<ComponentStorage<Tp>...> & t
    ) {
        auto & storage = std::get<I>(t);
        update_if_exists(scene, storage);

        if constexpr(I+1 != sizeof...(Tp))
            inner_update<I+1>(scene, t);
    }

    friend class Scene;
};

} // namespace prt3

#endif
