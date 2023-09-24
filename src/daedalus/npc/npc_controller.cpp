#include "npc_controller.h"
#include "src/daedalus/npc/npc_db.h"

using namespace dds;

void NPCController::on_init(prt3::Scene & /*scene*/) {}

void NPCController::on_update(prt3::Scene & scene, float delta_time) {
    update_action(scene, delta_time);
}

void NPCController::update_action(prt3::Scene & scene, float /*delta_time*/) {
    prt3::Armature & armature = scene.get_component<prt3::Armature>(node_id());
    prt3::Animation & anim =
        scene.animation_system().get_animation(armature.animation_id());
    prt3::Model const & model = scene.get_model(armature.model_handle());

    NPCDB & db = m_game_state->npc_db();

    NPCAction & action = db.peek_schedule(m_npc_id);

    switch (action.type) {
        case NPCAction::GO_TO_DESTINATION: {
            update_go_to_dest(scene);
            break;
        }
        case NPCAction::WAIT:
        case NPCAction::WAIT_UNTIL: {
            action.u.wait.duration -= dds::ms_per_frame;
            int32_t anim_index = model.get_animation_index("idle");
            if (anim.clip_a.animation_index != anim_index) {
                anim.clip_a.animation_index = anim_index;
                anim.clip_a.looping = true;
                anim.clip_a.speed = 1.0f;
                anim.clip_a.t = 0.0f;
                anim.blend_factor = 0.0f;
            }
            break;
        }
        default: {}
    }

    NPC & npc = db.get_npc(m_npc_id);
    npc.map_position.position =
        get_node(scene).get_global_transform(scene).position;
}

void NPCController::update_go_to_dest(prt3::Scene & scene) {
    NPCDB & db = m_game_state->npc_db();
    NPC & npc = db.get_npc(m_npc_id);

    prt3::Node & node = scene.get_node(node_id());

    /* move unconditionally */
    glm::vec3 new_pos = npc.map_position.position;

    prt3::Transform tform = node.get_global_transform(scene);
    glm::vec3 look_dir = new_pos - tform.position;
    if (look_dir != glm::vec3{0.0f}) {
        look_dir = glm::normalize(look_dir);
        tform.rotation = glm::rotation(glm::vec3{0.0f, 0.0f, 1.0f}, look_dir);
    }

    node.set_global_transform(scene, tform);

    /* push out of intersecting geometry */
    node.move_and_collide(scene, glm::vec3{0.0f});
}
