#include "player_controller.h"

using namespace prt3;

void PlayerController::exclude_player_from_scene_fade(Scene & scene) {
    thread_local std::vector<NodeID> queue;
    queue.emplace_back(node_id());
    while (!queue.empty()) {
        NodeID id = queue.back();
        queue.pop_back();

        scene.scene_manager().exclude_node_from_fade(id);

        for (NodeID child_id : scene.get_node(id).children_ids()) {
            queue.push_back(child_id);
        }
    }
}

void PlayerController::on_init(Scene & scene) {
    CharacterController::on_init(scene);
    set_tag(scene, "player");
    scene.selected_node() = node_id();

    exclude_player_from_scene_fade(scene);

    m_blob_shadow_id = scene.add_node_to_root("");
    scene.add_component<Decal>(m_blob_shadow_id);
    Decal & decal = scene.get_component<Decal>(m_blob_shadow_id);
    decal.texture_id() =
        scene.upload_texture("assets/textures/decals/blob_shadow.png");
    decal.dimensions() = glm::vec3{2.0f, 1.0f, 2.0f};
}

void PlayerController::update_input(Scene & scene, float /*delta_time*/) {
    Input & input = scene.get_input();

    m_state.input.run = input.get_key(KeyCode::KEY_CODE_LEFT_SHIFT);
    m_state.input.attack = input.get_key_down(KeyCode::KEY_CODE_ENTER);
    m_state.input.jump = input.get_key_down(KeyCode::KEY_CODE_SPACE);

    glm::vec2 raw_input_dir{0.0f};

    if (input.get_key(KeyCode::KEY_CODE_W)) {
        raw_input_dir += glm::vec2{0.0f, 1.0f};
    }
    if (input.get_key(KeyCode::KEY_CODE_S)) {
        raw_input_dir -= glm::vec2{0.0f, 1.0f};
    }
    if (input.get_key(KeyCode::KEY_CODE_A)) {
        raw_input_dir -= glm::vec2{1.0f, 0.0f};
    }
    if (input.get_key(KeyCode::KEY_CODE_D)) {
        raw_input_dir += glm::vec2{1.0f, 0.0f};
    }

    m_state.input.direction = glm::vec3{ 0.0f };
    // project input according to camera
    if (raw_input_dir != glm::vec2{0.0f}) {
        // compute look direction
        Camera const & camera = scene.get_camera();
        glm::vec3 c_proj_x = camera.get_right();
        constexpr float quarter_pi = glm::pi<float>() / 4.0f;
        glm::vec3 c_proj_y =
            glm::abs(glm::asin(-camera.get_front().y)) <= quarter_pi ?
            camera.get_front() :
            glm::sign(-camera.get_front().y) * camera.get_up();

        m_state.input.direction = glm::normalize(
            raw_input_dir.x * glm::vec3{c_proj_x.x, 0.0f, c_proj_x.z} +
            raw_input_dir.y * glm::vec3{c_proj_y.x, 0.0f, c_proj_y.z}
        );
    }

    if (m_state.grounded) {
        m_state.input.last_grounded_direction = m_state.input.direction;
        m_state.input.last_grounded_run = m_state.input.run;
        m_state.jump_count = 0;
        // project movement onto ground
        m_state.input.direction = m_state.input.direction -
                            glm::dot(m_state.input.direction,
                                        m_state.ground_normal) *
                                m_state.ground_normal;
        if (m_state.input.direction != glm::vec3{0.0f}) {
            m_state.input.direction = glm::normalize(m_state.input.direction);
        }
    }
}

void PlayerController::on_update(Scene & scene, float delta_time) {
    CharacterController::on_update(scene, delta_time);
    Transform tform = get_node(scene).get_global_transform(scene);

    ColliderTag tag =
        scene.get_component<ColliderComponent>(node_id()).tag();
    CollisionLayer mask = scene.physics_system().get_collision_mask(tag);

    Node & blob_shadow = scene.get_node(m_blob_shadow_id);

    float eps = 0.05f;
    RayHit hit;
    if (scene.physics_system().raycast(
        tform.position + glm::vec3{0.0f, eps, 0.0f},
        glm::vec3{0.0f, -1.0f, 0.0f},
        100.0f,
        mask,
        tag,
        hit
    )) {
        Decal & decal = scene.get_component<Decal>(m_blob_shadow_id);

        float diff_y = tform.position.y - hit.position.y;
        float dim_y = diff_y + 2.0f * eps;

        float t_xz = glm::clamp(0.5f * diff_y, 0.0f, 1.0f);
        float dim_xz = glm::mix(2.0f, 1.0f, t_xz);
        decal.dimensions() = glm::vec3{dim_xz, dim_y, dim_xz};

        float offset_y = -(0.5f * diff_y + eps);

        glm::vec3 blob_pos = tform.position;
        blob_pos.y += offset_y;
        blob_shadow.set_global_position(scene, blob_pos);
        blob_shadow.local_transform().scale = glm::vec3{1.0f};

        glm::vec3 up{0.0f, 1.0f, 0.0f};
        glm::vec3 u_dot_n = glm::cross(up, hit.normal);
        glm::vec3 axis = u_dot_n == glm::vec3{0.0f} ?
            up : glm::normalize(u_dot_n);

        float angle = acosf(glm::dot(up, hit.normal));

        blob_shadow.set_global_rotation(
            scene,
            glm::angleAxis(angle, axis)
        );
    } else {
        blob_shadow.local_transform().scale = glm::vec3{0.0f};
    }
}
