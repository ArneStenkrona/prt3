#include "npc_controller.h"

#include "src/daedalus/npc/npc_db.h"
#include "src/daedalus/game_state/game_state.h"

using namespace dds;

inline static bool is_running(NPCDB const & db, NPCID id) {
    npc_action::ActionType action_type = db.get_action_type(id);
    if (action_type == npc_action::GO_TO_DESTINATION) {
        return db.get_action<npc_action::GoToDest>(id).running;
    } else if (action_type == npc_action::FOLLOW) {
        return db.get_action<npc_action::Follow>(id).running;
    }
    return false;
}

inline static MapPathID path_id(NPCDB const & db, NPCID id) {
    npc_action::ActionType action_type = db.get_action_type(id);
    if (action_type == npc_action::GO_TO_DESTINATION) {
        return db.get_action<npc_action::GoToDest>(id).path_id;
    } else if (action_type == npc_action::FOLLOW) {
        return db.get_action<npc_action::Follow>(id).path_id;
    }
    return NO_MAP_PATH;
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

    m_walk_anim_speed_a = 2.5f;
    m_walk_anim_speed_b = 2.5f;
    m_run_anim_speed = 3.0f;

    m_walk_force = npc.walk_force * dds::time_scale;
    m_run_force = npc.run_force * dds::time_scale;

    npc_action::ActionType action_type = db.get_action_type(m_npc_id);

    // pre-init
    switch (action_type) {
        case npc_action::GO_TO_DESTINATION:
        case npc_action::FOLLOW: {
            m_state.state = is_running(db, m_npc_id) ?
                CharacterController::State::RUN :
                CharacterController::State::WALK;
            break;
        }
        case npc_action::WARP: {
            update_warp(scene, 0.0f);
            break;
        }
        default: {
        }
    }

    CharacterController::on_init(scene);

    // post-init
    switch (action_type) {
        case npc_action::GO_TO_DESTINATION:
        case npc_action::FOLLOW: {
            bool running = is_running(db, m_npc_id);
            float force = running ? m_run_force : m_walk_force;
            float speed = force / (npc.friction * dds::time_scale);
            m_state.velocity = speed * npc.direction;
            m_state.direction = npc.direction;
            m_state.input.direction = npc.direction;
            m_state.run_factor = running ? 2.0f : 1.0f;
            break;
        }
        default: {
        }
    }

    m_movement_performance = 1.0f;
}

void set_material_override(
    prt3::Scene & scene,
    prt3::NodeID node_id,
    prt3::MaterialOverride mat_override
) {
    thread_local std::vector<prt3::NodeID> queue;
    queue.push_back(node_id);
    while (!queue.empty()) {
        prt3::NodeID id = queue.back();
        prt3::Node const & node = scene.get_node(id);
        queue.pop_back();

        if (scene.has_component<prt3::MaterialComponent>(id)) {
            auto & mat = scene.get_component<prt3::MaterialComponent>(id);
            mat.material_override() = mat_override;
        }

        for (prt3::NodeID child_id : node.children_ids()) {
            queue.push_back(child_id);
        }
    }
}

void NPCController::on_update(prt3::Scene & scene, float delta_time) {
    NPCDB & db = m_game_state->npc_db();
    NPC & npc = db.get_npc(m_npc_id);

    m_walk_force = npc.walk_force * dds::time_scale;
    m_run_force = npc.run_force * dds::time_scale;

    CharacterController::on_update(scene, delta_time);

    prt3::Node & node = scene.get_node(node_id());

    /* move unconditionally */
    prt3::Transform tform = node.get_global_transform(scene);

    npc.map_position.position = tform.position;

    npc.friction = m_state.friction / dds::time_scale;

    npc_action::ActionType action_type = db.get_action_type(m_npc_id);

    if (action_type == npc_action::GO_TO_DESTINATION ||
        action_type == npc_action::FOLLOW) {
        float n = static_cast<float>(m_rolling_avg_n);
        float movement_performance =
            m_update_data.expected_move_distance == 0.0f ?
            1.0f :
            m_update_data.move_distance / m_update_data.expected_move_distance;
        m_movement_performance = m_movement_performance -
                                (m_movement_performance / n) +
                                (movement_performance / n);
    } else {
        m_movement_performance = 1.0f;
    }
}

void NPCController::update_input(prt3::Scene & scene, float delta_time) {
    NPCDB & db = m_game_state->npc_db();
    npc_action::ActionType action_type = db.get_action_type(m_npc_id);

    m_state.input.attack = false;
    m_state.input.cast_spell = false;

    switch (action_type) {
        case npc_action::GO_TO_DESTINATION:
        case npc_action::FOLLOW: {
            update_moving(scene, delta_time);
            break;
        }
        case npc_action::WARP: {
            update_warp(scene, delta_time);
            m_state.input.direction = glm::vec3{0.0f};
            break;
        }
        case npc_action::ATTACK: {
            update_attack(scene, delta_time);
            break;
        }
        case npc_action::USE_ITEM: {
            update_use_item(scene, delta_time);
            break;
        }
        default: {
            m_state.input.direction = glm::vec3{0.0f};
        }
    }
}

void NPCController::update_moving(prt3::Scene & /*scene*/, float delta_time) {
    NPCDB & db = m_game_state->npc_db();
    NPC & npc = db.get_npc(m_npc_id);

    m_state.input.run = is_running(db, m_npc_id);

    glm::vec3 target_dir = npc.direction;
    target_dir.y = 0.0f;

    if (path_id(db, m_npc_id) == NO_MAP_PATH) {
        m_state.input.direction = glm::vec3{0.0f};
    } else if (target_dir != glm::vec3{0.0f}) {
        target_dir = glm::normalize(target_dir);
        m_state.input.direction = GameState::smooth_change_dir(
            m_state.input.direction,
            target_dir,
            40.0f,
            delta_time
        );
    }

    if (m_movement_performance < 0.2f) {
        db.set_stuck(m_npc_id);
        db.queue_clear_action(m_npc_id);
    }
}

void NPCController::update_warp(prt3::Scene & scene, float /*delta_time*/) {
    NPCDB & db = m_game_state->npc_db();
    NPC & npc = db.get_npc(m_npc_id);
    npc_action::Warp & warp = db.get_action<npc_action::Warp>(m_npc_id);

    float alpha;
    switch (warp.phase) {
        case npc_action::Warp::Phase::fade_in: {
            alpha = 1.0f -
                glm::clamp(float(warp.timer) / warp.fade_time, 0.0f, 1.0f);
            break;
        }
        case npc_action::Warp::Phase::fade_out: {
            alpha = glm::clamp(float(warp.timer) / warp.fade_time, 0.0f, 1.0f);
            break;
        }
    }

    get_node(scene).set_global_position(scene, npc.map_position.position);

    prt3::MaterialOverride mo;
    mo.tint_active = true;
    mo.tint = glm::vec4{1.0f, 1.0f, 1.0f, alpha};
    set_material_override(scene, node_id(), mo);
}

void NPCController::update_attack(prt3::Scene & scene, float delta_time) {
    NPCDB & db = m_game_state->npc_db();
    npc_action::Attack & attack = db.get_action<npc_action::Attack>(m_npc_id);

    if (attack.activated) {
        return;
    }
    attack.activated = true;

    prt3::Node & node = scene.get_node(node_id());

    prt3::Transform tform = node.get_global_transform(scene);

    glm::vec3 target = db.get_target_position(scene, attack.target).position;
    glm::vec3 dir = target - tform.position;
    dir.y = 0.0f;
    if (dir != glm::vec3{0.0f}) {
        dir = glm::normalize(dir);
        m_state.input.direction = GameState::smooth_change_dir(
            m_state.input.direction,
            dir,
            40.0f,
            delta_time
        );
    }

    m_state.input.attack = true;
}

void NPCController::update_use_item(prt3::Scene & scene, float /*delta_time*/) {
    NPCDB & db = m_game_state->npc_db();
    npc_action::UseItem & use_item =
        db.get_action<npc_action::UseItem>(m_npc_id);

    if (use_item.activated && m_state.state == CAST_SPELL) {
        prt3::Armature & armature =
            scene.get_component<prt3::Armature>(node_id());
        prt3::Animation & anim =
            scene.animation_system().get_animation(armature.animation_id());

        float frac = anim.clip_a.frac();
        if (frac > 0.5f) {
            db.game_state().item_db().use(
                scene,
                m_npc_id,
                use_item.item,
                use_item.target
            );
            db.queue_clear_action(m_npc_id);
        }

        return;
    }
    if (m_state.state != CAST_SPELL &&
        m_game_state->item_db().is_spell(use_item.item)) {
        use_item.activated = true;
        m_state.input.cast_spell = true;
    }

    if (m_state.state != CAST_SPELL &&
        m_game_state->item_db().is_spell(use_item.item) &&
        m_game_state->current_time() - db.get_action_timestamp(m_npc_id) >
            dds::ms_per_frame * 60) {
        /* Couldn't set character set, give up */
        db.queue_clear_action(m_npc_id);
        return;
    }
}
