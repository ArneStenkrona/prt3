#ifndef PRT3_SCENE_H
#define PRT3_SCENE_H

#include "src/engine/scene/node.h"
#include "src/engine/scene/transform_cache.h"
#include "src/engine/component/component_manager.h"
#include "src/engine/component/script_set.h"
#include "src/engine/component/script/script.h"
#include "src/engine/physics/physics_system.h"
#include "src/engine/rendering/renderer.h"
#include "src/engine/rendering/camera.h"
#include "src/engine/core/input.h"

#include <vector>

namespace prt3
{

class Context;

class Scene {
public:
    Scene(Context & context);
    ~Scene();

    Scene(Scene const & other) = delete;
    Scene & operator=(Scene const & other) = delete;

    NodeID add_node(NodeID parent_id);
    NodeID add_node_to_root() { return add_node(m_root_id); }
    NodeID get_next_available_node_id() const { return m_nodes.size(); }

    void set_node_mesh(NodeID node_id, ResourceID mesh_id)
        { m_component_manager.set_mesh_component(node_id, mesh_id); }
    void set_node_material(NodeID node_id, ResourceID material_id)
        { m_component_manager.set_material_component(node_id, material_id); }
    void set_node_point_light(NodeID node_id, PointLight const & light)
        { m_component_manager.set_point_light_component(node_id, light); }

    void set_node_collider(NodeID node_id, Sphere const & sphere)
        { m_physics_system.add_sphere_collider(node_id, sphere); }

    void set_node_local_position(NodeID node_id, glm::vec3 const & local_position)
        { m_nodes[node_id].m_local_transform.position = local_position; }

    void set_directional_light(DirectionalLight light) { m_directional_light = light; }
    void set_directional_light_on(bool on) { m_directional_light_on = on; }
    void set_ambient_light(glm::vec3 color) { m_ambient_light.color = color; }

    Node & get_node(NodeID id) { return m_nodes[id]; }
    Camera & get_camera() { return m_camera; }
    Input & get_input();

    Script * get_script(ScriptID id) { return m_scripts[id]; }

    template<typename ComponentType>
    ComponentType & add_component(NodeID id) {
        return m_component_manager.add_component<ComponentType>(id);
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
    bool has_component(NodeID id) {
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

private:
    Context & m_context;

    Camera m_camera;

    NodeID m_root_id;
    std::vector<Node> m_nodes;

    std::vector<Script *> m_scripts;
    std::vector<Script *> m_init_queue;


    ComponentManager m_component_manager;
    PhysicsSystem m_physics_system;

    DirectionalLight m_directional_light;
    bool m_directional_light_on = false;

    AmbientLight m_ambient_light;

    TransformCache m_transform_cache;

    void update(float delta_time);
    void render();
    void collect_render_data(RenderData & render_data) const;
    void update_window_size(int w, int h);

    // template<class T>
    // ScriptID internal_add_script(NodeID node_id) {
    //     ScriptID id = m_scripts.size();
    //     T * script = new T(*this, node_id);
    //     m_scripts.push_back(static_cast<Script *>(script));
    //     m_init_queue.push_back(static_cast<Script *>(script));
    //     return id;
    // }
    ScriptID internal_add_script(Script * script) {
        ScriptID id = m_scripts.size();
        m_scripts.push_back(script);
        m_init_queue.push_back(script);
        return id;
    }

    Script * internal_get_script(ScriptID id) {
        return m_scripts[id];
    }

    friend class Engine;
    friend class Renderer;
    friend class Node;
    friend class ScriptSet;
};

} // namespace prt3

#endif
