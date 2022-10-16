#include "scene.h"

#include "src/engine/rendering/render_data.h"
#include "src/engine/core/context.h"

#include "src/engine/component/script/camera_controller.h"
#include "src/engine/component/script/character_controller.h"

#include <algorithm>

using namespace prt3;

Scene::Scene(Context & context)
 : m_context{context},
   m_camera{context.input(),
            context.renderer().window_width(),
            context.renderer().window_height()},
   m_component_manager{*this},
   m_physics_system{*this} {
    m_root_id = m_nodes.size();
    m_nodes.emplace_back(m_root_id, *this);

    // FOR DEBUGGING, WILL REMOVE ---->
    NodeID island = m_context.model_manager()
        .add_model_to_scene_from_path(
            "assets/models/test_room/test_room.fbx",
            *this,
            m_root_id
        );

    m_physics_system.add_mesh_collider(
        island,
        "assets/models/test_room/test_room.fbx"
    );

    set_ambient_light(glm::vec3{0.1f, 0.1f, 0.1f});

    PostProcessingPass outline_pass_info;
    outline_pass_info.fragment_shader_path =
        "assets/shaders/opengl/pixel_postprocess.fs";
    outline_pass_info.downscale_factor =
        m_context.renderer().downscale_factor();
    PostProcessingPass upscale_pass_info;
    upscale_pass_info.fragment_shader_path =
        "assets/shaders/opengl/passthrough.fs";
    upscale_pass_info.downscale_factor = 1.0f;

    m_context.renderer().set_postprocessing_chain(
        {outline_pass_info, upscale_pass_info}
    );

    // set_directional_light({{0.0f, -1.0f, 0.0f}, {0.8f, 0.8f, 0.8f}});
    // set_directional_light_on(true);

    NodeID cam_node = add_node_to_root();
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

    NodeID light_id = add_node(character);
    get_node(light_id).local_transform().position = glm::vec3{0.0f, 7.0f, 0.0f};
    PointLight light;
    light.color = glm::vec3(1.0f, 1.0f, 1.0f);
    light.quadratic_term = 0.02f;
    light.linear_term = 0.01f;
    light.constant_term = 0.1f;
    m_component_manager.set_point_light_component(light_id, light);

    add_script<CharacterController>(character);
    m_physics_system.add_sphere_collider(character, {{0.0f, 2.1f, 0.0f}, 0.9f});

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

    render();

    m_physics_system.update(
        m_transform_cache.global_transforms().data(),
        m_transform_cache.global_transforms_history().data()
    );
}

void Scene::render() {
    static RenderData render_data;
    render_data.clear();

    collect_render_data(render_data);

    m_context.renderer().render(render_data);
}

NodeID Scene::add_node(NodeID parent_id) {
    Node & parent = m_nodes[parent_id];
    NodeID id = m_nodes.size();
    parent.m_children_ids.push_back(id);

    m_nodes.emplace_back(id, *this);
    m_nodes[id].m_parent_id = parent_id;

    return id;
}

Input & Scene::get_input() {
    return m_context.input();
}

void Scene::collect_render_data(RenderData & render_data) const {
    /* scene data */
    render_data.scene_data.view_matrix = m_camera.get_view_matrix();
    render_data.scene_data.projection_matrix = m_camera.get_projection_matrix();
    render_data.scene_data.view_position = m_camera.get_position();
    render_data.scene_data.view_direction = m_camera.get_front();
    render_data.scene_data.near_plane = m_camera.near_plane();
    render_data.scene_data.far_plane = m_camera.far_plane();

    std::vector<Transform> const & global_transforms =
        m_transform_cache.global_transforms();

    auto material_components = m_component_manager.get_material_components();
    for (auto const & pair : m_component_manager.get_mesh_components()) {
        MeshRenderData mesh_data;
        mesh_data.mesh_id = pair.second;
        mesh_data.material_id = NO_RESOURCE;
        mesh_data.transform = global_transforms[pair.first].to_matrix();
        if (material_components.find(pair.first) != material_components.end()) {
            mesh_data.material_id = material_components[pair.first];
        }
        render_data.mesh_data.push_back(mesh_data);
    }

    std::vector<PointLightRenderData> point_lights;
    for (auto const & pair : m_component_manager.get_point_light_components()) {
        PointLightRenderData point_light_data;
        point_light_data.light = pair.second;
        point_light_data.position = global_transforms[pair.first].to_matrix()
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
