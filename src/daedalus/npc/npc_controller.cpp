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

    prt3::Armature & armature = scene.get_component<prt3::Armature>(node_id());
    prt3::Animation & anim =
        scene.animation_system().get_animation(armature.animation_id());
    prt3::Model const & model = scene.get_model(armature.model_handle());

    int32_t anim_index;

    NPCDB & db = m_game_state->npc_db();
    NPCAction::ActionType action_type = db.schedule_empty(m_npc_id) ?
        NPCAction::ActionType::NONE : db.peek_schedule(m_npc_id).type;
    switch (action_type) {
        case NPCAction::GO_TO_DESTINATION: {
            anim_index = model.get_animation_index("walk");
            break;
        }
        default: {
            anim_index = model.get_animation_index("idle");
        }
    }

    glm::vec3 dir = db.get_npc(m_npc_id).direction;
    if (dir != glm::vec3{0.0f}) {
        prt3::Node & node = get_node(scene);
        prt3::Transform tform = node.get_global_transform(scene);

        glm::quat rot = glm::rotation(glm::vec3{0.0f, 0.0f, 1.0f}, dir);
        rot = glm::slerp(tform.rotation, rot, 1.0f);
        node.set_global_rotation(scene, rot);
    }

    set_anim_if_not_set(anim, anim_index);
}

void NPCController::on_update(prt3::Scene & scene, float delta_time) {
    NPCDB & db = m_game_state->npc_db();
    if (!db.schedule_empty(m_npc_id)) {
        update_action(scene, delta_time);
    }
}

void NPCController::update_action(prt3::Scene & scene, float delta_time) {
    prt3::Armature & armature = scene.get_component<prt3::Armature>(node_id());
    prt3::Animation & anim =
        scene.animation_system().get_animation(armature.animation_id());
    prt3::Model const & model = scene.get_model(armature.model_handle());

    NPCDB & db = m_game_state->npc_db();

    NPCAction & action = db.peek_schedule(m_npc_id);

    switch (action.type) {
        case NPCAction::GO_TO_DESTINATION: {
            update_go_to_dest(scene, delta_time);
            int32_t anim_index = model.get_animation_index("walk");
            set_anim_if_not_set(anim, anim_index);
            break;
        }
        case NPCAction::WAIT:
        case NPCAction::WAIT_UNTIL: {
            action.u.wait.duration -= dds::ms_per_frame;
            int32_t anim_index = model.get_animation_index("idle");
            set_anim_if_not_set(anim, anim_index);
            break;
        }
        default: {}
    }

    NPC & npc = db.get_npc(m_npc_id);
    npc.map_position.position =
        get_node(scene).get_global_transform(scene).position;
}

void NPCController::update_go_to_dest(prt3::Scene & scene, float delta_time) {
    NPCDB & db = m_game_state->npc_db();
    NPC & npc = db.get_npc(m_npc_id);

    prt3::Node & node = scene.get_node(node_id());

    /* move unconditionally */
    prt3::Transform tform = node.get_global_transform(scene);

    glm::vec3 target_pos = npc.map_position.position;
    glm::vec3 delta_pos = target_pos - tform.position;

    glm::vec3 look_dir = delta_pos;
    look_dir.y = 0.0f;
    if (look_dir != glm::vec3{0.0f}) {
        look_dir = glm::normalize(look_dir);
        glm::quat target_rot = glm::rotation(glm::vec3{0.0f, 0.0f, 1.0f}, look_dir);

        float speed = 20.0f;
        float dt_fac = 1.0f - glm::pow(0.5f, delta_time * speed);
        glm::quat rot = glm::slerp(tform.rotation, target_rot, dt_fac);

        node.set_global_rotation(scene, rot);
    }

    /* push out of intersecting geometry */
    node.move_and_collide(scene, delta_pos);
}
