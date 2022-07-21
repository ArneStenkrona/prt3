#include "scene.h"

#include "src/engine/rendering/render_data.h"
#include "src/engine/core/context.h"

#include <algorithm>

using namespace prt3;

Scene::Scene(Context & context)
 : m_context{context},
   m_camera{context.input(), context.renderer().window_width(), context.renderer().window_height()} {
    m_root_id = m_nodes.size();
    m_nodes.push_back({});

    // FOR DEBUGGING, WILL REMOVE ---->
    m_context.model_manager()
        .add_model_to_scene_from_path("assets/models/debug/pbr_cube.fbx", *this, m_root_id);

    NodeID light_node = add_node_to_root();
    PointLight light;
    light.color = glm::vec3(1.0f, 1.0f, 1.0f);
    light.quadratic_term = 0.01f;
    light.linear_term = 0.01f;
    light.constant_term = 0.0f;
    m_component_manager.set_point_light_component(light_node, light);
    set_node_local_position(light_node, glm::vec3(2.1f, 2.1f, 2.1f));

    m_context.renderer().set_postprocesing_shader("assets/shaders/opengl/sinusoidal.fs");
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
    render_data.scene_data.view_position = m_camera.get_position();

    /* mesh data */
    struct QueueElement {
        NodeID node_id;
        glm::mat4 global_transform;
    };

    std::vector<QueueElement> queue{{m_root_id,
                                     m_nodes[m_root_id].local_transform.to_transform_matrix()}};

    std::unordered_map<NodeID, glm::mat4> global_transforms;

    while (!queue.empty()) {
        NodeID node_id = queue.back().node_id;
        Node const & node = m_nodes[node_id];
        glm::mat4 node_transform = queue.back().global_transform;
        global_transforms.insert({node_id, node_transform});
        queue.pop_back();

        for (NodeID const & child_id : node.children) {
            Node const & child = m_nodes[child_id];
            glm::mat4 child_transform =
                child.local_transform.to_transform_matrix() * node_transform;
            queue.push_back({child_id, child_transform});
        }
    }

    auto material_components = m_component_manager.get_material_components();
    for (auto const & pair : m_component_manager.get_mesh_components()) {
        MeshRenderData mesh_data;
        mesh_data.mesh_id = pair.second;
        mesh_data.material_id = NO_RESOURCE;
        mesh_data.transform = global_transforms[pair.first];
        if (material_components.find(pair.first) != material_components.end()) {
            mesh_data.material_id = material_components[pair.first];
        }
        render_data.mesh_data.push_back(mesh_data);
    }

    std::vector<PointLightRenderData> point_lights;
    for (auto const & pair : m_component_manager.get_point_light_components()) {
        PointLightRenderData point_light_data;
        point_light_data.light = pair.second;
        point_light_data.position = global_transforms[pair.first]
                                        * glm::vec4(0.0f,0.0f,0.0f,1.0f);

        point_lights.push_back(point_light_data);
    }

    glm::vec3 camera_position = m_camera.get_position();
    std::sort(point_lights.begin(), point_lights.end(),
        [&camera_position](PointLightRenderData const & a, PointLightRenderData const & b) {
            return glm::distance2(a.position, camera_position) <
                   glm::distance2(b.position, camera_position);
        });

    render_data.light_data.number_of_point_lights =
        point_lights.size() < LightRenderData::MAX_NUMBER_OF_POINT_LIGHTS ?
            point_lights.size() : LightRenderData::MAX_NUMBER_OF_POINT_LIGHTS;
    for (size_t i = 0; i < render_data.light_data.number_of_point_lights; ++i) {
        render_data.light_data.point_lights[i] = point_lights[i];
    }
}

void Scene::update_window_size(int w, int h) {
    m_camera.set_size(w, h);
}
