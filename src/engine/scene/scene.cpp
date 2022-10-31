#include "scene.h"

#include "src/engine/core/context.h"

#include "src/engine/component/script/camera_controller.h"
#include "src/engine/component/script/character_controller.h"

#include <algorithm>

using namespace prt3;

Scene::Scene(Context & context)
 : m_context{context},
   m_camera{context.renderer().window_width(),
            context.renderer().window_height()},
   m_component_manager{*this},
   m_physics_system{*this} {
    m_root_id = m_nodes.size();

    m_nodes.emplace_back(m_root_id, *this);
    m_node_names.emplace_back("root");

    // FOR DEBUGGING, WILL REMOVE ---->
    NodeID room = m_context.model_manager()
        .add_model_to_scene_from_path(
            "assets/models/test_room/test_room.fbx",
            *this,
            m_root_id
        );

    Model room_model{"assets/models/test_room/test_room.fbx"};
    add_component<ColliderComponent>(room, room_model);

    set_ambient_light(glm::vec3{0.1f, 0.1f, 0.1f});

    // set_directional_light({{0.0f, -1.0f, 0.0f}, {0.8f, 0.8f, 0.8f}});
    // set_directional_light_on(true);

    NodeID cam_node = add_node_to_root("camera");
    set_node_local_position(cam_node, glm::vec3(2.1f, 2.1f, 2.1f));
    ScriptID cam_controller = add_script<CameraController>(cam_node);

    NodeID character = m_context.model_manager()
        .add_model_to_scene_from_path(
            "assets/models/stranger/stranger.fbx",
            *this,
            m_root_id
        );
    get_node(character).local_transform().scale = glm::vec3(0.45f);
    get_node(character).local_transform().position.y = 1.0f;

    NodeID light_id = add_node(character, "point_light");
    get_node(light_id).local_transform().position = glm::vec3{0.0f, 7.0f, 0.0f};
    PointLight light;
    light.color = glm::vec3(1.0f, 1.0f, 1.0f);
    light.quadratic_term = 0.02f;
    light.linear_term = 0.01f;
    light.constant_term = 0.1f;
    add_component<PointLightComponent>(light_id, light);

    add_script<CharacterController>(character);
    Sphere sphere{{0.0f, 2.1f, 0.0f}, 0.9f};
    add_component<ColliderComponent>(character, sphere);

    get_script<CameraController>(cam_controller)->set_target(character);

    NodeID cube = m_context.model_manager()
        .add_model_to_scene_from_path("assets/models/debug/character_cube.fbx",
        *this,
        m_root_id
    );
    get_node(cube).set_global_position(glm::vec3(2.0f, 0.0f, 0.0f));

    m_physics_system.add_sphere_collider(cube, {{0.0f, 1.0f, 0.0f}, 0.9f});

    m_transform_cache.collect_global_transforms(
        m_nodes.data(),
        m_nodes.size(),
        m_root_id
    );
    // <---- FOR DEBUGGING, WILL REMOVE

}

Scene::~Scene() {
    for (Script * script : m_scripts) {
        delete script;
    }
}

void Scene::update(float delta_time) {
    m_physics_system.update(
        m_transform_cache.global_transforms().data(),
        m_transform_cache.global_transforms_history().data()
    );

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

    m_transform_cache.collect_global_transforms(
        m_nodes.data(),
        m_nodes.size(),
        m_root_id
    );
}

void Scene::update_transform_cache() {
    m_transform_cache.collect_global_transforms(
        m_nodes.data(),
        m_nodes.size(),
        m_root_id
    );
}

NodeID Scene::add_node(NodeID parent_id, const char * name) {
    Node & parent = m_nodes[parent_id];
    NodeID id = m_nodes.size();
    parent.m_children_ids.push_back(id);

    m_nodes.emplace_back(id, *this);
    m_nodes[id].m_parent_id = parent_id;

    m_node_names.emplace_back(name);

    return id;
}

Input & Scene::get_input() {
    return m_context.input();
}

void Scene::collect_world_render_data(
    WorldRenderData & world_data,
    NodeID selected
) const {
    std::vector<Transform> const & global_transforms =
        m_transform_cache.global_transforms();

    static std::unordered_set<NodeID> selected_incl_children;
    selected_incl_children.clear();
    static std::vector<NodeID> queue;
    queue.push_back(selected);
    while (!queue.empty()) {
        NodeID curr = queue.back();
        Node const & curr_node = get_node(curr);
        queue.pop_back();
        selected_incl_children.insert(curr);

        for (NodeID const & child_id : curr_node.children_ids()) {
            queue.push_back(child_id);
        }
    }

    auto const & mesh_comps = m_component_manager.get_all_components<Mesh>();
    for (auto const & mesh_comp : mesh_comps) {
        NodeID id = mesh_comp.node_id();
        MeshRenderData mesh_data;
        mesh_data.mesh_id = mesh_comp.resource_id();
        mesh_data.material_id = NO_RESOURCE;
        mesh_data.node = id;

        mesh_data.transform = global_transforms[id].to_matrix();
        mesh_data.material_id = get_component<Material>(id).resource_id();
        world_data.mesh_data.push_back(mesh_data);
        if (selected_incl_children.find(id) !=
            selected_incl_children.end()) {
            world_data.selected_mesh_data.push_back(mesh_data);
        }
    }

    std::vector<PointLightRenderData> point_lights;
    auto const & lights
        = m_component_manager.get_all_components<PointLightComponent>();
    for (auto const & light : lights) {
        PointLightRenderData point_light_data;
        point_light_data.light = light.light();
        point_light_data.position = global_transforms[light.node_id()].to_matrix()
                                        * glm::vec4(0.0f,0.0f,0.0f,1.0f);

        point_lights.push_back(point_light_data);
    }

    glm::vec3 camera_position = m_camera.get_position();
    std::sort(point_lights.begin(), point_lights.end(),
        [&camera_position](PointLightRenderData const & a, PointLightRenderData const & b) {
            return glm::distance2(a.position, camera_position) <
                   glm::distance2(b.position, camera_position);
        });

    world_data.light_data.number_of_point_lights =
        point_lights.size() < LightRenderData::MAX_NUMBER_OF_POINT_LIGHTS ?
            point_lights.size() : LightRenderData::MAX_NUMBER_OF_POINT_LIGHTS;
    for (size_t i = 0; i < world_data.light_data.number_of_point_lights; ++i) {
        world_data.light_data.point_lights[i] = point_lights[i];
    }

    world_data.light_data.directional_light = m_directional_light;
    world_data.light_data.directional_light_on = m_directional_light_on;

    world_data.light_data.ambient_light = m_ambient_light;
}

void Scene::update_window_size(int w, int h) {
    m_camera.set_size(w, h);
}

void Scene::emit_signal(SignalString const & signal, void * data) {
    for (Script * script : m_signal_connections[signal]) {
        script->on_signal(signal, data);
    }
}
