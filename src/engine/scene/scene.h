#ifndef PRT3_SCENE_H
#define PRT3_SCENE_H

#include "src/engine/scene/node.h"
#include "src/engine/scene/component_manager.h"
#include "src/engine/scene/script/script.h"
#include "src/engine/rendering/renderer.h"
#include "src/engine/rendering/camera.h"

#include <vector>
#include <type_traits>

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

    template<class T>
    void add_script(NodeID node_id) {
        T * script = new T(*this, node_id);
        m_scripts.push_back(static_cast<Script *>(script));
        m_init_queue.push_back(static_cast<Script *>(script));
    }

    void set_node_mesh(NodeID node_id, ResourceID mesh_id)
        { m_component_manager.set_mesh_component(node_id, mesh_id); }
    void set_node_material(NodeID node_id, ResourceID material_id)
        { m_component_manager.set_material_component(node_id, material_id); }
    void set_node_point_light(NodeID node_id, PointLight const & light)
        { m_component_manager.set_point_light_component(node_id, light); }

    void set_node_local_position(NodeID node_id, glm::vec3 const & local_position)
        { m_nodes[node_id].m_local_transform.position = local_position; }

    void set_directional_light(DirectionalLight light) { m_directional_light = light; }
    void set_directional_light_on(bool on) { m_directional_light_on = on; }
    void set_ambient_light(glm::vec3 color) { m_ambient_light.color = color; }

    Node & get_node(NodeID id) { return m_nodes[id]; }
    Camera & get_camera() { return m_camera; }

private:
    Context & m_context;

    Camera m_camera;

    NodeID m_root_id;
    std::vector<Node> m_nodes;
    std::vector<Script *> m_scripts;
    std::vector<Script *> m_init_queue;

    ComponentManager m_component_manager;

    DirectionalLight m_directional_light;
    bool m_directional_light_on = false;

    AmbientLight m_ambient_light;

    void update(float delta_time);
    void render();
    void collect_render_data(RenderData & render_data) const;
    void update_window_size(int w, int h);

    friend class Engine;
    friend class Renderer;
};

} // namespace prt3

#endif
