#ifndef PRT3_SCENE_H
#define PRT3_SCENE_H

#include "src/engine/scene/node.h"
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
        { m_nodes[node_id].mesh_id = mesh_id; }
    void set_node_material(NodeID node_id, ResourceID material_id)
        { m_nodes[node_id].material_id = material_id; }

private:
    Context & m_context;

    Camera m_camera;

    NodeID m_root_id;
    std::vector<Node> m_nodes;

    void update(float delta_time);
    void render();
    void collect_render_data(RenderData & render_data) const;

    friend class Engine;
};

} // namespace prt3

#endif