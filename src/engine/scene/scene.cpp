#include "scene.h"

#include "src/engine/rendering/render_data.h"
#include "src/engine/core/context.h"

#include "src/engine/scene/script/camera_controller.h"
#include "src/engine/scene/script/character_controller.h"

#include <algorithm>

using namespace prt3;

Scene::Scene(Context & context)
 : m_context{context},
   m_camera{context.input(), context.renderer().window_width(), context.renderer().window_height()} {
    m_root_id = m_nodes.size();
    m_nodes.emplace_back(*this);

    // FOR DEBUGGING, WILL REMOVE ---->
    m_context.model_manager()
        .add_model_to_scene_from_path("assets/models/moon_island/moon_island.fbx", *this, m_root_id);

    NodeID light_node = add_node_to_root();
    PointLight light;
    light.color = glm::vec3(1.0f, 1.0f, 1.0f);
    light.quadratic_term = 0.1f;
    light.linear_term = 0.2f;
    light.constant_term = 0.1f;
    m_component_manager.set_point_light_component(light_node, light);
    set_node_local_position(light_node, glm::vec3(2.1f, 2.1f, 2.1f));

    set_ambient_light(glm::vec3{0.09f, 0.11f, 0.34f});

    PostProcessingPass outline_pass_info;
    outline_pass_info.fragment_shader_path = "assets/shaders/opengl/pixel_postprocess.fs";
    outline_pass_info.downscale_factor = m_context.renderer().downscale_factor();
    PostProcessingPass upscale_pass_info;
    upscale_pass_info.fragment_shader_path = "assets/shaders/opengl/passthrough.fs";
    upscale_pass_info.downscale_factor = 1.0f;

    m_context.renderer().set_postprocessing_chain(
        {outline_pass_info, upscale_pass_info}
    );

    set_directional_light({{1.0f, -1.0f, -1.0f}, {0.8f, 0.8f, 0.8f}});
    set_directional_light_on(true);

    CameraController * cam_controller = add_script<CameraController>(light_node);

    NodeID character = m_context.model_manager()
        .add_model_to_scene_from_path("assets/models/debug/character_cube.fbx", *this, m_root_id);

    add_script<CharacterController>(character);

    cam_controller->set_target(character);
    // <---- FOR DEBUGGING, WILL REMOVE

}

Scene::~Scene() {
    for (Script * script : m_scripts) {
        delete script;
    }
}

void Scene::update(float delta_time) {
    for (Script * script : m_init_queue) {
        script->on_init();
    }
    m_init_queue.clear();

    for (Script * script : m_scripts) {
        script->on_update(delta_time);
    }

    for (Script * script : m_scripts) {
        script->on_late_update(delta_time);
    }

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
    parent.m_children_ids.push_back(id);

    m_nodes.emplace_back(*this);
    m_nodes[id].m_parent_id = parent_id;

    return id;
}

Input & Scene::get_input() {
    return m_context.input();
}

void Scene::collect_render_data(RenderData & render_data) const {
    assert(render_data.mesh_data.size() = 0);
    /* scene data */
    render_data.scene_data.view_matrix = m_camera.get_view_matrix();
    render_data.scene_data.projection_matrix = m_camera.get_projection_matrix();
    render_data.scene_data.view_position = m_camera.get_position();
    render_data.scene_data.view_direction = m_camera.get_front();
    render_data.scene_data.near_plane = m_camera.near_plane();
    render_data.scene_data.far_plane = m_camera.far_plane();

    /* mesh data */
    struct QueueElement {
        NodeID node_id;
        glm::mat4 global_transform;
    };

    std::vector<QueueElement> queue{{m_root_id,
                                     m_nodes[m_root_id].m_local_transform.to_matrix()}};

    std::unordered_map<NodeID, glm::mat4> global_transforms;

    while (!queue.empty()) {
        NodeID node_id = queue.back().node_id;
        Node const & node = m_nodes[node_id];
        glm::mat4 node_transform = queue.back().global_transform;
        global_transforms.insert({node_id, node_transform});
        queue.pop_back();

        for (NodeID const & child_id : node.children_ids()) {
            Node const & child = m_nodes[child_id];
            glm::mat4 child_transform =
                child.m_local_transform.to_matrix() * node_transform;
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

    render_data.light_data.directional_light = m_directional_light;
    render_data.light_data.directional_light_on = m_directional_light_on;

    render_data.light_data.ambient_light = m_ambient_light;
}

void Scene::update_window_size(int w, int h) {
    m_camera.set_size(w, h);
}
