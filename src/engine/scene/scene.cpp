#include "scene.h"

#include "src/engine/rendering/render_data.h"
#include "src/engine/core/context.h"

using namespace prt3;

Scene::Scene(Context & context)
 : m_context{context} {
    m_root_id = m_nodes.size();
    m_nodes.push_back({});
}

void Scene::render() {
    std::vector<RenderData> render_data;
    collect_render_data(render_data);

    m_context.renderer().render(render_data);
}

NodeID Scene::add_node(NodeID parent_id) {
    Node & parent = m_nodes[parent_id];
    NodeID id = m_nodes.size();
    parent.children.push_back(id);

    m_nodes.push_back({});
    m_nodes[id].parent = parent_id;

    return id;
}


void Scene::collect_render_data(std::vector<RenderData> & render_data) const {
    struct QueueElement {
        NodeID node_id;
        glm::mat4 global_transform;
    };

    std::vector<QueueElement> queue{{m_root_id,
                                     m_nodes[m_root_id].local_transform.to_transform_matrix()}};

    while (!queue.empty()) {
        Node const & node = m_nodes[queue.back().node_id];
        glm::mat4 node_transform = queue.back().global_transform;
        queue.pop_back();

        if (node.should_be_rendered()) {
            RenderData data;
            data.mesh_rid = node.mesh_id;
            data.material_rid = node.material_id;
            data.transform = node_transform;

            render_data.push_back(data);
        }

        for (NodeID const & child_id : node.children)
        {
            Node const & child = m_nodes[child_id];
            glm::mat4 child_transform =
                    child.local_transform.to_transform_matrix() * node_transform;
            queue.push_back({child_id, child_transform});
        }
    }
}
