#ifndef PRT3_SCENE_H
#define PRT3_SCENE_H

#include "src/engine/scene/node.h"
#include "src/engine/scene/script_container.h"
#include "src/engine/scene/signal.h"
#include "src/engine/scene/transform_cache.h"
#include "src/engine/component/component_manager.h"
#include "src/engine/component/script_set.h"
#include "src/engine/component/armature.h"
#include "src/engine/component/animated_mesh.h"
#include "src/engine/component/script/script.h"
#include "src/engine/physics/physics_system.h"
#include "src/engine/animation/animation_system.h"
#include "src/engine/rendering/renderer.h"
#include "src/engine/rendering/camera.h"
#include "src/engine/core/input.h"
#include "src/util/uuid.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace prt3 {

class Context;
class Editor;
class SceneManager;

class Scene {
public:
    Scene(Context & context);

    void serialize(std::ostream & out) const;
    void deserialize(std::istream & in);

    void serialize_components(std::ostream & out, NodeID id) const
    { m_component_manager.serialize_components(out, *this, id); }

    void deserialize_components(std::istream & in, NodeID id)
    { m_component_manager.deserialize_components(in, *this, id); }

    template<typename ComponentType>
    void serialize_component(std::ostream & out, NodeID id) const
    { m_component_manager.serialize_component<ComponentType>(out, *this, id); }

    template<typename ComponentType>
    void deserialize_component(std::istream & in, NodeID id)
    { m_component_manager.deserialize_component<ComponentType>(in, *this, id); }

    void clear() { internal_clear(true); }

    NodeID add_node(NodeID parent_id, NodeName name)
    { return add_node(parent_id, name.data(), generate_uuid()); }
    NodeID add_node(NodeID parent_id, const char * name)
    { return add_node(parent_id, name, generate_uuid()); }
    NodeID add_node_to_root(const char * name) { return add_node(s_root_id, name); }
    NodeID get_next_available_node_id() const
    { return m_free_list.empty() ? m_nodes.size() : m_free_list.back(); }

    ModelHandle upload_model(std::string const & path)
    { return register_model(model_manager().upload_model(path)); }

    ModelHandle add_model_to_scene_from_path(
        std::string const & path,
        NodeID parent_id
    ) {  return register_model(model_manager()
        .add_model_to_scene_from_path(path, *this, parent_id));
    }

    NodeID get_root_id() const { return s_root_id; }

    bool remove_node(NodeID id);

    void set_node_local_position(NodeID node_id, glm::vec3 const & local_position)
    { m_nodes[node_id].m_local_transform.position = local_position; }

    void set_directional_light(DirectionalLight light) { m_directional_light = light; }
    void set_directional_light_on(bool on) { m_directional_light_on = on; }
    void set_ambient_light(glm::vec3 color) { m_ambient_light.color = color; }

    Node const & get_node(NodeID id) const { return m_nodes[id]; }
    Node & get_node(NodeID id) { return m_nodes[id]; }
    NodeID get_node_id_from_uuid(UUID uuid) const
    { return m_uuid_to_node.at(uuid); };
    UUID get_uuid_from_node_id(NodeID id) const
    { return m_node_uuids.at(id); };
    bool node_exists(NodeID id) const
    { return m_nodes.size() > static_cast<size_t>(id) && m_nodes[id].id() != NO_NODE; }
    std::string get_node_path(NodeID id) const;

    Camera & get_camera() { return m_camera; }
    Input & get_input();

    Script const * get_script(ScriptID id) const
    { return m_script_container.scripts().at(id); }
    Script * get_script(ScriptID id)
    { return m_script_container.scripts().at(id); }

    template<typename ComponentType, typename... ArgTypes>
    ComponentType & add_component(NodeID id, ArgTypes... args) {
        return m_component_manager.add_component<ComponentType>(*this, id, args...);
    }

    template<typename ComponentType>
    bool remove_component(NodeID id) {
        return m_component_manager.remove_component<ComponentType>(*this, id);
    }

    /**
     * Note: reference should be considered stale if
     *       any components of same type is added
     *       or removed
     */
    template<typename ComponentType>
    ComponentType & get_component(NodeID id) {
        return m_component_manager.get_component<ComponentType>(id);
    }

    template<typename ComponentType>
    ComponentType const & get_component(NodeID id) const {
        return m_component_manager.get_component<ComponentType>(id);
    }

    template<typename ComponentType>
    std::vector<ComponentType> & get_all_components() {
        return m_component_manager.get_all_components<ComponentType>();
    }

    template<typename ComponentType>
    std::vector<ComponentType> const & get_all_components() const {
        return m_component_manager.get_all_components<ComponentType>();
    }

    template<typename ComponentType>
    bool has_component(NodeID id) const {
        return m_component_manager.has_component<ComponentType>(id);
    }

    template<typename T>
    ScriptID add_script(NodeID id) {
        auto & man = m_component_manager;
        if (!man.has_component<ScriptSet>(id)) {
            man.add_component<ScriptSet>(*this, id);
        }
        return man.get_component<ScriptSet>(id).add_script<T>(*this);
    }

    template<typename T>
    T * get_script(ScriptID id) {
        return dynamic_cast<T*>(m_script_container.scripts().at(id));
    }

    template<typename T>
    T const * get_script(ScriptID id) const {
        return dynamic_cast<T const *>(m_script_container.scripts().at(id));
    }

    void remove_script(ScriptID id) { m_script_container.remove(id); }

    void connect_signal(SignalString const & signal,
                        Script * script) {
        m_signal_connections[signal].insert(script);
    }

    void emit_signal(SignalString const & signal, void * data);

    bool add_tag_to_node(NodeTag const & tag, NodeID id) {
        if (m_tags.find(tag) != m_tags.end()) {
            return false;
        }
        m_tags.emplace(std::make_pair(tag, id));
        m_node_to_tag.emplace(std::make_pair(id, tag));
        return true;
    }

    NodeID find_node_by_tag(NodeTag const & tag) const {
        if (m_tags.find(tag) != m_tags.end()) {
            return m_tags.at(tag);
        }
        return NO_NODE;
    }

    NodeName const & get_node_name(NodeID id) const { return m_node_names[id]; }
    NodeName & get_node_name(NodeID id) { return m_node_names[id]; }

    // Mod flags are cleared just before scripts are updated
    Node::ModFlags get_node_mod_flags(NodeID id) const
    { return m_node_mod_flags[id]; }

    // Mod flags are cleared just before scripts are updated
    bool get_node_mod_flag(NodeID id, Node::ModFlags flag) const
    { return m_node_mod_flags[id] & flag; }

    PhysicsSystem const & physics_system() const { return m_physics_system; }
    PhysicsSystem & physics_system() { return m_physics_system; }

    AnimationSystem const & animation_system() const { return m_animation_system; }
    AnimationSystem & animation_system() { return m_animation_system; }

    ModelManager const & model_manager() const;
    Model const & get_model(ModelHandle handle) const;

    std::unordered_set<ModelHandle> const & referenced_models() const
    { return m_referenced_models; }

    SceneManager & scene_manager();

    inline std::vector<NodeID> const & get_overlaps(NodeID node_id) const
    { return m_physics_system.get_overlaps(node_id); }

private:
    Context * m_context;

    Camera m_camera;

    static constexpr NodeID s_root_id = 0;
    std::vector<Node> m_nodes;
    std::vector<NodeName> m_node_names;
    std::vector<Node::ModFlags> m_node_mod_flags;
    std::vector<NodeID> m_free_list;
    std::unordered_map<NodeID, UUID> m_node_uuids;
    std::unordered_map<UUID, NodeID> m_uuid_to_node;

    ScriptContainer m_script_container;

    std::unordered_map<SignalString, std::unordered_set<Script *> >
        m_signal_connections;

    std::unordered_map<NodeTag, NodeID> m_tags;
    std::unordered_map<NodeID, NodeTag> m_node_to_tag;

    ComponentManager m_component_manager;
    PhysicsSystem m_physics_system;
    AnimationSystem m_animation_system;

    DirectionalLight m_directional_light;
    bool m_directional_light_on = false;

    AmbientLight m_ambient_light;

    TransformCache m_transform_cache;

    std::unordered_set<ModelHandle> m_referenced_models;

    NodeID add_node(NodeID parent_id, const char * name, UUID uuid);

    ModelHandle register_model(ModelHandle handle)
    { m_referenced_models.insert(handle); return handle; }

    void update(float delta_time);

    void clear_node_mod_flags();

    void collect_world_render_data(
        WorldRenderData & world_data,
        NodeID selected
    );

    void update_window_size(int w, int h);

    ScriptID internal_add_script(Script * script) {
        return m_script_container.add_script(script);
    }

    Script const * internal_get_script(ScriptID id) const {
        return m_script_container.scripts().at(id);
    }

    Script * internal_get_script(ScriptID id) {
        return m_script_container.scripts().at(id);
    }

    void internal_remove_script(ScriptID id) {
        m_script_container.remove(id);
    }

    void internal_clear(bool place_root);

    void mark_ancestors(NodeID id, Node::ModFlags flag) {
        NodeID ancestor_id = m_nodes[id].parent_id();
        while (ancestor_id != NO_NODE) {
            m_node_mod_flags[ancestor_id] =
                m_node_mod_flags[ancestor_id] | flag;
            ancestor_id = m_nodes[ancestor_id].parent_id();
        }
    }

    ModelManager & model_manager();

    friend class Engine;
    friend class Renderer;
    friend class Node;
    friend class ScriptSet;
    friend class Editor;
    friend class EditorContext;
    friend class Armature;
    friend AnimatedModel::AnimatedModel(Scene &, NodeID, std::istream &);
    friend ModelComponent::ModelComponent(Scene &, NodeID, std::istream &);
    friend MaterialComponent::MaterialComponent(Scene &, NodeID, std::istream &);
    friend Mesh::Mesh(Scene &, NodeID, std::istream &);
    friend AnimatedMesh::AnimatedMesh(Scene &, NodeID, std::istream &);
};

} // namespace prt3

#endif
