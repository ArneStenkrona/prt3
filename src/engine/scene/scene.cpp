#include "scene.h"

#include "src/engine/rendering/render_data.h"
#include "src/engine/core/context.h"

using namespace prt3;

Scene::Scene(Context & context)
 : m_context{context},
   m_camera{context.input(), context.renderer().window_width(), context.renderer().window_height()} {
    m_root_id = m_nodes.size();
    m_nodes.push_back({});

        // FOR DEBUGGING, WILL REMOVE ---->
    m_context.model_manager()
        .add_model_to_scene_from_path("assets/models/debug/cube_lattice.fbx", *this, m_root_id);
    // <---- FOR DEBUGGING, WILL REMOVE
}

void Scene::update(float delta_time) {
    m_camera.update(delta_time, true, true);
    render();
}

void Scene::render() {
    RenderData render_data;
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


void Scene::collect_render_data(RenderData & render_data) const {
    assert(render_data.mesh_data.size() = 0);
    /* scene data */
    render_data.scene_data.view_matrix = m_camera.get_view_matrix();
    render_data.scene_data.projection_matrix = m_camera.get_projection_matrix();

    /* mesh data */
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
            MeshRenderData mesh_data;
            mesh_data.mesh_id = node.mesh_id;
            mesh_data.material_id = node.material_id;
            mesh_data.transform = node_transform;

            render_data.mesh_data.push_back(mesh_data);
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

void Scene::update_window_size(int w, int h) {
    m_camera.set_size(w, h);
}
