#include "npc_controller.h"
#include "src/daedalus/npc/npc_db.h"

using namespace dds;

void set_anim_if_not_set(prt3::Animation & anim, int32_t anim_index) {
    if (anim.clip_a.animation_index != anim_index) {
        anim.clip_a.animation_index = anim_index;
        anim.clip_a.looping = true;
        anim.clip_a.speed = 1.0f;
        anim.clip_a.t = 0.0f;
        anim.blend_factor = 0.0f;
    }
}

void NPCController::on_init(prt3::Scene & scene) {
    m_game_state = scene.get_autoload_script<GameState>();
    NPCDB & db = m_game_state->npc_db();
    NPC & npc = db.get_npc(m_npc_id);

    /* Temporary workaround due to the fact that the nav mesh is usually
     * hovering a bit above ground. The better solution is to update nav mesh
     * generation so that the mesh snaps to the actual ground.
     */
    prt3::ColliderTag tag =
        scene.get_component<prt3::ColliderComponent>(node_id()).tag();
    prt3::CollisionLayer mask = scene.physics_system().get_collision_mask(tag);

    float eps = 0.1f;

    glm::vec3 pos = get_node(scene).get_global_transform(scene).position;
    pos += npc.direction * eps;

    prt3::RayHit hit;
    if (scene.physics_system().raycast(
        pos + glm::vec3{0.0f, eps, 0.0f},
        glm::vec3{0.0f, -1.0f, 0.0f},
        2.0f,
        mask,
        tag,
        hit
    )) {
        get_node(scene).set_global_position(scene, hit.position);
    }

    m_walk_anim_speed_a = 1.0f;
    m_walk_anim_speed_b = 1.0f;
    m_run_anim_speed = 1.25f;

    m_walk_force = npc.walk_force * dds::time_scale;
    m_run_force = npc.run_force * dds::time_scale;

    NPCAction::ActionType action_type = db.schedule_empty(m_npc_id) ?
    NPCAction::NONE : db.peek_schedule(m_npc_id).type;

    // pre-init
    switch (action_type) {
        case NPCAction::GO_TO_DESTINATION: {
            NPCAction & action = db.peek_schedule(m_npc_id);
            auto & gtd = action.u.go_to_dest;
            m_state.state = gtd.running ?
                CharacterController::State::RUN :
                CharacterController::State::WALK;
            break;
        }
        default: {
        }
    }

    CharacterController::on_init(scene);

    // post-init
    switch (action_type) {
        case NPCAction::GO_TO_DESTINATION: {
            NPCAction & action = db.peek_schedule(m_npc_id);
            auto & gtd = action.u.go_to_dest;
            float force = gtd.running ? m_run_force : m_walk_force;
            float speed = force / (npc.friction * dds::time_scale);
            m_state.velocity = speed * npc.direction;
            m_state.direction = npc.direction;
            m_state.input.direction = npc.direction;
            m_state.run_factor = gtd.running ? 2.0f : 1.0f;
            break;
        }
        default: {
        }
    }
}

void NPCController::on_update(prt3::Scene & scene, float delta_time) {
    NPCDB & db = m_game_state->npc_db();
    NPC & npc = db.get_npc(m_npc_id);

    m_walk_force = npc.walk_force * dds::time_scale;
    m_run_force = npc.run_force * dds::time_scale;

    prt3::CharacterController::on_update(scene, delta_time);

    prt3::Node & node = scene.get_node(node_id());

    /* move unconditionally */
    prt3::Transform tform = node.get_global_transform(scene);

    npc.map_position.position = tform.position;

    npc.friction = m_state.friction / dds::time_scale;
}

void NPCController::update_input(prt3::Scene & scene, float delta_time) {
    NPCDB & db = m_game_state->npc_db();
    NPCAction & action = db.peek_schedule(m_npc_id);

    switch (action.type) {
        case NPCAction::GO_TO_DESTINATION: {
            update_go_to_dest(scene, delta_time);
            break;
        }
        default: {
            m_state.input.direction = glm::vec3{0.0f};
        }
    }
}


void NPCController::update_go_to_dest(prt3::Scene & scene, float delta_time) {
    NPCDB & db = m_game_state->npc_db();
    NPC & npc = db.get_npc(m_npc_id);
    NPCAction & action = db.peek_schedule(m_npc_id);

    prt3::Node & node = scene.get_node(node_id());

    /* move unconditionally */
    prt3::Transform tform = node.get_global_transform(scene);

    glm::vec3 target_pos = npc.map_position.position;
    glm::vec3 delta_pos = target_pos - tform.position;
    delta_pos.y = 0.0f;

    m_state.input.run = true;

    glm::vec3 target_dir = npc.direction;
    target_dir.y = 0.0f;

    if (action.u.go_to_dest.path_id == NO_MAP_PATH) {
        ++m_missing_path_count;
    } else {
        m_missing_path_count = 0;
    }

    if (m_missing_path_count > 1) {
        m_state.input.direction = glm::vec3{0.0f};
    } else if (target_dir != glm::vec3{0.0f}) {
        target_dir = glm::normalize(target_dir);

        float speed = 40.0f;
        float dt_fac = 1.0f - glm::pow(0.5f, delta_time * speed);
        m_state.input.direction = glm::normalize(
            glm::mix(m_state.input.direction, target_dir, dt_fac)
        );
    }
}
