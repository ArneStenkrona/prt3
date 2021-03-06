#ifndef PRT3_SCENE_H
#define PRT3_SCENE_H

#include "src/engine/scene/node.h"
#include "src/engine/scene/component_manager.h"
#include "src/engine/rendering/renderer.h"
#include "src/engine/rendering/camera.h"

#include <vector>

namespace prt3
{

class Context;

class Scene {
public:
    Scene(Context & context);

    NodeID add_node(NodeID parent_id);
    NodeID add_node_to_root() { return add_node(m_root_id); }

    void set_node_mesh(NodeID node_id, ResourceID mesh_id)
        { m_component_manager.set_mesh_component(node_id, mesh_id); }
    void set_node_material(NodeID node_id, ResourceID material_id)
        { m_component_manager.set_material_component(node_id, material_id); }
    void set_node_point_light(NodeID node_id, PointLight const & light)
        { m_component_manager.set_point_light_component(node_id, light); }

    void set_node_local_position(NodeID node_id, glm::vec3 const & local_position)
        { m_nodes[node_id].local_transform.position = local_position; }

private:
    Context & m_context;

    Camera m_camera;

    NodeID m_root_id;
    std::vector<Node> m_nodes;

    ComponentManager m_component_manager;

    void update(float delta_time);
    void render();
    void collect_render_data(RenderData & render_data) const;
    void update_window_size(int w, int h);

    friend class Engine;
    friend class Renderer;
};

} // namespace prt3

#endif
