#include "npcs.h"

#include "src/daedalus/npc/npc_db.h"
#include "src/daedalus/game_state/game_state.h"

using namespace dds;

void npc_update_test(
    NPCID id,
    NPCDB & npc_db,
    prt3::Scene & scene
) {
    NPC & npc = npc_db.get_npc(id);

    GameState & game_state = npc_db.game_state();

    prt3::Node const & player = scene.get_node(game_state.player_id());
    glm::vec3 player_pos = player.get_global_transform(scene).position;

    TimeMS time = game_state.current_time();
    if (npc_db.has_no_action(id)) {
        if ((npc.stuck && time - npc.stuck_since > 1000 * dds::time_scale)) {
            npc_action::Warp warp;
            warp.fade_time = 1000 * dds::time_scale;
            warp.timer = 0;
            warp.destination.position = player_pos;
            warp.destination.room = game_state.current_room();
            warp.phase = npc_action::Warp::Phase::fade_in;
            npc_db.queue_action<npc_action::Warp>(id, std::move(warp));
        } else {
            AnyID target;
            target.type = IDType::dds_id_type_player;
            npc_action::Follow follow;
            follow.target = target;
            follow.path_id = NO_MAP_PATH;
            follow.path_threshold = 0.5f;
            follow.target_dist = 1.0f;
            follow.stop_on_arrival = false;
            follow.running = true;
            npc_db.queue_action<npc_action::Follow>(id, std::move(follow));
        }
    }

    if (npc_db.get_action_type(id) == npc_action::FOLLOW) {
        float dist = glm::distance(npc.map_position.position, player_pos);
        // float arcane_threshold = 12.5f;
        float melee_threshold = 3.5f;

        // if (dist < arcane_threshold &&
        //     npc_db.game_state().item_db().ready_to_use(
        //         id, ItemID::item_spell_flame_pillar
        // )) {
        //     npc_action::UseItem use_item;
        //     use_item.item = ItemID::item_spell_flame_pillar;
        //     use_item.target.type = IDType::dds_id_type_player;
        //     use_item.activated = false;
        //     npc_db.queue_action<npc_action::UseItem>(id, std::move(use_item));
        // } else if (dist < arcane_threshold &&
        //     npc_db.game_state().item_db().ready_to_use(
        //         id, ItemID::item_spell_fire_rock
        // )) {
        //     npc_action::UseItem use_item;
        //     use_item.item = ItemID::item_spell_fire_rock;
        //     use_item.target.type = IDType::dds_id_type_player;
        //     use_item.activated = false;
        //     npc_db.queue_action<npc_action::UseItem>(id, std::move(use_item));
        // } else if (dist < melee_threshold) {
        //     if (npc_db.has_no_action(id) ||
        //         npc_db.get_action_type(id) == npc_action::FOLLOW) {
        //         npc_action::Attack attack;
        //         attack.target.id = 0; // d/c for player
        //         attack.target.type = IDType::dds_id_type_player;
        //         attack.timer = 0;
        //         attack.activated = false;
        //         npc_db.queue_action<npc_action::Attack>(id, std::move(attack));
        //     }
        // }
        if (dist < melee_threshold) {
            if (npc_db.has_no_action(id) ||
                npc_db.get_action_type(id) == npc_action::FOLLOW) {
                npc_action::Attack attack;
                attack.target.id = 0; // d/c for player
                attack.target.type = IDType::dds_id_type_player;
                attack.timer = 0;
                attack.activated = false;
                npc_db.queue_action<npc_action::Attack>(id, std::move(attack));
            }
        }
    }
}

void create_wraith(NPCDB & db) {
    NPCID npc_id = db.push_npc();

    NPC & npc = db.get_npc(npc_id);
    npc.map_position.room = db.game_state().map().get_room_id_from_num(0);
    npc.map_position.position = glm::vec3{-4.0f, 0.5f, -0.5f};
    npc.model_path = "assets/models/boss1/boss1.fbx";
    npc.model_scale = 0.62f;
    npc.prefab_id = PrefabDB::dark_flames;
    npc.hand_equip_r_prefab_id = PrefabDB::wraith_sword;
    npc.collider_radius = 1.0f;
    npc.collider_height = 3.75f;
    npc.walk_force = 50.0f / dds::time_scale;
    npc.run_force = 118.0f / dds::time_scale;

    npc.mode = NPC::Mode::active;
    npc.update = npc_update_test;
}

void dds::create_npcs(NPCDB & db) {
    create_wraith(db);
}
