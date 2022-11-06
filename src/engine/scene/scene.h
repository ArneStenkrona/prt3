#ifndef PRT3_SCENE_H
#define PRT3_SCENE_H

#include "src/engine/scene/node.h"
#include "src/engine/scene/signal.h"
#include "src/engine/scene/transform_cache.h"
#include "src/engine/component/component_manager.h"
#include "src/engine/component/script_set.h"
#include "src/engine/component/script/script.h"
#include "src/engine/physics/physics_system.h"
#include "src/engine/rendering/renderer.h"
#include "src/engine/rendering/camera.h"
#include "src/engine/core/input.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace prt3 {

class Context;

class Scene {
public:
    Scene(Context & context);
    ~Scene();

    void serialize(std::ostream & out) const;
    void deserialize(std::istream & in);
    void clear() { internal_clear(true); }

    Scene(Scene const & other) = delete;
    Scene & operator=(Scene const & other) = delete;

    NodeID add_node(NodeID parent_id, const char * name);
    NodeID add_node_to_root(const char * name) { return add_node(m_root_id, name); }
    NodeID get_next_available_node_id() const { return m_nodes.size(); }

    NodeID get_root_id() const { return m_root_id; }

    void set_node_local_position(NodeID node_id, glm::vec3 const & local_position)
        { m_nodes[node_id].m_local_transform.position = local_position; }

    void set_directional_light(DirectionalLight light) { m_directional_light = light; }
    void set_directional_light_on(bool on) { m_directional_light_on = on; }
    void set_ambient_light(glm::vec3 color) { m_ambient_light.color = color; }

    Node const & get_node(NodeID id) const { return m_nodes[id]; }
    Node & get_node(NodeID id) { return m_nodes[id]; }
    Camera & get_camera() { return m_camera; }
    Input & get_input();

    Script const * get_script(ScriptID id) const { return m_scripts[id]; }
    Script * get_script(ScriptID id) { return m_scripts[id]; }

    template<typename ComponentType, typename... ArgTypes>
    ComponentType & add_component(NodeID id, ArgTypes... args) {
        return m_component_manager.add_component<ComponentType>(id, args...);
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
    bool has_component(NodeID id) const {
        return m_component_manager.has_component<ComponentType>(id);
    }

    template<typename T>
    ScriptID add_script(NodeID id) {
        auto & man = m_component_manager;
        if (!man.has_component<ScriptSet>(id)) {
            man.add_component<ScriptSet>(id);
        }
        return man.get_component<ScriptSet>(id).add_script<T>();
    }

    template<typename T>
    T * get_script(ScriptID id) {
        return dynamic_cast<T*>(m_scripts[id]);
    }

    template<typename T>
    T const * get_script(ScriptID id) const {
        return dynamic_cast<T const *>(m_scripts[id]);
    }

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

    PhysicsSystem const & physics_system() const { return m_physics_system; }
    PhysicsSystem & physics_system() { return m_physics_system; }

    ModelManager const & model_manager() const;

private:
    Context & m_context;

    Camera m_camera;

    static constexpr NodeID m_root_id = 0;
    std::vector<Node> m_nodes;
    std::vector<NodeName> m_node_names;

    std::vector<Script *> m_scripts;
    std::vector<Script *> m_init_queue;
    std::vector<Script *> m_late_init_queue;

    std::unordered_map<SignalString, std::unordered_set<Script *> >
        m_signal_connections;

    std::unordered_map<NodeTag, NodeID> m_tags;
    std::unordered_map<NodeID, NodeTag> m_node_to_tag;

    ComponentManager m_component_manager;
    PhysicsSystem m_physics_system;

    DirectionalLight m_directional_light;
    bool m_directional_light_on = false;

    AmbientLight m_ambient_light;

    TransformCache m_transform_cache;

    void update(float delta_time);
    void collect_world_render_data(
        WorldRenderData & world_data,
        NodeID selected
    ) const;
    void update_transform_cache();
    void update_window_size(int w, int h);

    ScriptID internal_add_script(Script * script) {
        ScriptID id = m_scripts.size();
        m_scripts.push_back(script);
        m_init_queue.push_back(script);
        m_late_init_queue.push_back(script);
        return id;
    }

    Script const * internal_get_script(ScriptID id) const {
        return m_scripts[id];
    }

    Script * internal_get_script(ScriptID id) {
        return m_scripts[id];
    }

    void internal_clear(bool place_root);

    ModelManager & model_manager();

    friend class Engine;
    friend class Renderer;
    friend class Node;
    friend class ScriptSet;
    friend class EditorContext;
    friend Material::Material(Scene &, NodeID, std::istream &);
    friend Mesh::Mesh(Scene &, NodeID, std::istream &);
};

} // namespace prt3

#endif
