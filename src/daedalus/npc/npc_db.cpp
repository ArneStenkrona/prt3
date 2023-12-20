#include "npc_db.h"

#include "src/daedalus/game_state/game_state.h"
#include "src/daedalus/npc/npc_controller.h"

using namespace dds;

// void attack_target(NPCID id, NPCDB & db) {
//     MapPosition pos;
//     MapPosition target_pos;
//     float dist;

//     float melee_threshold = 3.0f;
//     float projectile_threshold = 7.0f;

//     if (dist < melee_threshold) {
//         NPCAction action;
//         action.type = NPCAction::ATTACK;
//         auto const & attack = action.u.attack;
//         use_item.target.id = 0; // d/c for player
//         use_item.target.type = IDType::player;
//         db.push_schedule(id, action);
//     } else if (dist < projectile_threshold && use_spell) {
//         while (!db.schedule_empty(id)) {
//             db.pop_schedule(id, ScheduleStatus::success);
//         }

//         NPCAction action;
//         action.type = NPCAction::USE_ITEM;
//         auto const & use_item = action.u.use_item;
//         use_item.item = ItemDB::spell_flame_pillar;
//         AnyID target;
//         use_item.target.id = 0; // d/c for player
//         use_item.target.type = IDType::player;
//         db.push_schedule(id, action);
//     }
// }

char const * NPCAction::action_type_to_str(NPCAction::ActionType type) {
    switch (type) {
        case NPCAction::GO_TO_DESTINATION: return "GO_TO_DESTINATION";
        case NPCAction::FOLLOW: return "FOLLOW";
        case NPCAction::WARP: return "WARP";
        case NPCAction::WAIT: return "WAIT";
        case NPCAction::WAIT_UNTIL: return "WAIT_UNTIL";
        case NPCAction::USE_ITEM: return "USE_ITEM";
        case NPCAction::ATTACK: return "ATTACK";
        case NPCAction::NONE: return "NONE";
    }
}

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
    if (npc_db.schedule_empty(id)) {
        if ((npc.stuck && time - npc.stuck_since > 1000 * dds::time_scale)) {
            NPCAction action;
            action.type = NPCAction::WARP;
            action.u.warp.fade_time = 1000 * dds::time_scale;
            action.u.warp.timer = 0;
            action.u.warp.destination.position = player_pos;
            action.u.warp.destination.room = game_state.current_room();
            action.u.warp.phase = NPCAction::U::Warp::Phase::fade_in;
            npc_db.push_schedule(id, action);
        } else {
            NPCAction action;
            action.type = NPCAction::FOLLOW;
            AnyID target;
            target.type = IDType::dds_id_type_player;
            action.u.follow.target = target;
            action.u.follow.path_id = NO_MAP_PATH;
            action.u.follow.path_threshold = 0.5f;
            action.u.follow.target_dist = 1.0f;
            action.u.follow.stop_on_arrival = false;
            action.u.follow.running = true;
            npc_db.push_schedule(id, action);
        }
    }

    float dist = glm::distance(npc.map_position.position, player_pos);
    float melee_threshold = 3.5f;
    if (dist < melee_threshold) {
        while (!npc_db.schedule_empty(id) &&
               npc_db.peek_schedule(id).type == NPCAction::FOLLOW) {
            npc_db.pop_schedule(id, ScheduleStatus::success);
        }

        if (npc_db.schedule_empty(id)) {
            NPCAction action;
            action.type = NPCAction::ATTACK;
            auto & attack = action.u.attack;
            attack.target.id = 0; // d/c for player
            attack.target.type = IDType::dds_id_type_player;
            attack.timer = 0;
            attack.activated = false;
            npc_db.push_schedule(id, action);
        }
    }
}

NPCDB::NPCDB(GameState & game_state)
 : m_game_state{game_state} {
    /* for testing */
    NPCID npc_id = push_npc();

    NPC & npc = m_npcs[npc_id];
    npc.map_position.room = 4;
    npc.map_position.position = glm::vec3{-4.0f, 0.5f, -0.5f};
    npc.model_path = "assets/models/boss1/boss1.fbx";
    npc.model_scale = glm::vec3{0.62f};
    npc.prefab_id = PrefabDB::dark_flames;
    npc.collider_radius = 1.0f;
    npc.collider_length = 3.75f;
    npc.walk_force = 39.0f / dds::time_scale;
    npc.run_force = 90.0f / dds::time_scale;

    npc.mode = NPC::Mode::active;
    npc.update = npc_update_test;
}

void NPCDB::on_scene_start(prt3::Scene & scene) {
    load_npcs(scene);
}

void NPCDB::update(prt3::Scene & scene) {
    load_npcs(scene);

    for (auto it = m_loaded_npcs.begin();
         it != m_loaded_npcs.end();) {
        NPC & npc = m_npcs[it->first];
        if (npc.map_position.room != m_game_state.current_room()) {
            /* unload NPC */
            prt3::NodeID node_id = it->second;
            scene.remove_node(node_id);
            m_loaded_npcs.erase(it++);
        } else {
            ++it;
        }
    }

    for (NPCID id = 0; id < m_npcs.size(); ++id) {
        update_npc(id, scene);
    }
}

void NPCDB::on_scene_exit() {
    m_loaded_npcs.clear();
}

void NPCDB::pop_schedule(
    NPCID id,
    ScheduleStatus status
) {
    m_schedules[id].pop();

    NPC & npc = m_npcs[id];
    switch (status) {
        case ScheduleStatus::no_path_found:
        case ScheduleStatus::stuck_on_path: {
            if (!npc.stuck) {
                npc.stuck_since = m_game_state.current_time();
            }
            npc.stuck = true;
            break;
        }
        default: {
            npc.stuck = false;
        }
    }
}

MapPosition NPCDB::get_target_position(
    prt3::Scene const & scene,
    AnyID target
) const {
    switch (target.type) {
        case IDType::dds_id_type_player: {
            MapPosition res;
            res.room = m_game_state.current_room();
            prt3::Node const & p = scene.get_node(m_game_state.player_id());
            res.position = p.get_global_transform(scene).position;
            return res;
        }
        case IDType::dds_id_type_npc: {
            return m_npcs[target.id].map_position;
        }
        case IDType::dds_id_type_object: {
            /* TODO: implement */
            MapPosition res;
            res.position = glm::vec3{0.0f};
            return res;
        }
        case IDType::dds_id_type_item: {
            // nonsensical
            MapPosition res;
            res.position = glm::vec3{std::numeric_limits<float>::infinity()};
            return res;
        }
    }
}


NPCID NPCDB::push_npc() {
    NPCID id = m_npcs.size();
    m_npcs.push_back({});
    m_schedules.push_back({});
    return id;
}

void NPCDB::load_npc(prt3::Scene & scene, NPCID id) {
    NPC const & npc = m_npcs[id];

    prt3::NodeID node_id = scene.add_node_to_root("NPC");
    scene.add_model_to_scene_from_path(npc.model_path, node_id, true, true);

    prt3::Node & node = scene.get_node(node_id);
    node.set_global_position(scene, npc.map_position.position);
    node.set_global_scale(scene, npc.model_scale);

    prt3::Capsule capsule{};
    capsule.radius = npc.collider_radius;
    capsule.start.y = npc.collider_radius;
    capsule.end.y = npc.collider_radius + npc.collider_length;
    scene.add_component<prt3::ColliderComponent>(
        node_id,
        prt3::ColliderType::collider,
        capsule
    );

    prt3::AnimationID anim_id =
        scene.get_component<prt3::Armature>(node_id).animation_id();

    prt3::ScriptSet & script_set = scene.add_component<prt3::ScriptSet>(node_id);
    script_set.add_script<NPCController>(scene, id);

    m_loaded_npcs[id] = node_id;

    scene.animation_system().update_transforms(scene, anim_id);

    if (npc.prefab_id != PrefabDB::none) {
        prt3::Prefab const & prefab =
            m_game_state.prefab_db().get(npc.prefab_id);
        prefab.instantiate(scene, node_id);
    }
}

void NPCDB::load_npcs(prt3::Scene & scene) {
    for (NPCID id = 0; id < m_npcs.size(); ++id) {
        NPC & npc = m_npcs[id];
        if (m_loaded_npcs.find(id) == m_loaded_npcs.end()) {
            if (npc.map_position.room == m_game_state.current_room()) {
                load_npc(scene, id);
            }
        }
    }
}

void NPCDB::update_go_to_dest(
    NPCID id,
    NPCAction::U::GoToDest & data
) {
    NPC & npc = m_npcs[id];
    Map & map = m_game_state.map();

    if (!map.has_map_path(data.path_id)) {
        data.path_id = map.query_map_path(npc.map_position, data.destination);
    }

    if (data.path_id == NO_MAP_PATH) {
        pop_schedule(id, ScheduleStatus::no_path_found);
        return;
    }

    float force = data.running ? npc.run_force : npc.walk_force;
    float velocity = (force / npc.friction);
    float length = velocity * dds::frame_dt_over_time_scale;

    bool arrived = map.advance_map_path(
        data.path_id,
        npc.map_position.position,
        length,
        npc.map_position,
        npc.direction
    );

    if (arrived) {
        pop_schedule(id);
    }
}

void NPCDB::update_follow(
    prt3::Scene const & scene,
    NPCID id,
    NPCAction::U::Follow & data
) {
    NPC & npc = m_npcs[id];
    Map & map = m_game_state.map();

    MapPosition target_pos = get_target_position(scene, data.target);
    float dist = glm::distance(npc.map_position.position, target_pos.position);

    bool has_path = map.has_map_path(data.path_id);
    bool gen_path = !has_path;
    if (has_path) {
        float dest_dist = glm::distance(
            map.get_map_destination(data.path_id).position,
            target_pos.position
        );
        gen_path |= dest_dist > data.path_threshold;
    }

    if (gen_path) {
        data.path_id = map.query_map_path(npc.map_position, target_pos);
    }

    if (data.path_id == NO_MAP_PATH) {
        pop_schedule(id, ScheduleStatus::no_path_found);
        return;
    }

    if (dist > data.target_dist) {
        float force = data.running ? npc.run_force : npc.walk_force;
        float velocity = (force / npc.friction);
        float length = velocity * dds::frame_dt_over_time_scale;

        bool arrived = map.advance_map_path(
            data.path_id,
            npc.map_position.position,
            length,
            npc.map_position,
            npc.direction
        );

        if (arrived && data.stop_on_arrival) {
            pop_schedule(id);
        }
    }
}

void NPCDB::update_warp(
    NPCID id,
    NPCAction::U::Warp & data
) {
    NPC & npc = m_npcs[id];

    bool complete = false;

    switch (data.phase) {
        case NPCAction::U::Warp::Phase::fade_in: {
            if (data.timer >= data.fade_time) {
                data.phase = NPCAction::U::Warp::Phase::fade_out;
                data.timer = 0;
                npc.map_position = data.destination;
            }
            break;
        }
        case NPCAction::U::Warp::Phase::fade_out: {
            if (data.timer >= data.fade_time) {
                complete = true;
            }
            break;
        }
    }

    data.timer += dds::ms_per_frame;

    if (complete) {
        pop_schedule(id);
    }
}

void NPCDB::update_npc(NPCID id, prt3::Scene & scene) {
    bool should_update =
        schedule_empty(id) || m_npcs[id].mode == NPC::Mode::active;

    if (should_update) {
        m_npcs[id].update(id, *this, scene);
    }

    if (schedule_empty(id)) {
        return;
    }

    NPCAction & action = peek_schedule(id);

    switch (action.type) {
        case NPCAction::GO_TO_DESTINATION: {
            update_go_to_dest(id, action.u.go_to_dest);
            break;
        }
        case NPCAction::FOLLOW: {
            update_follow(scene, id, action.u.follow);
            break;
        }
        case NPCAction::WARP: {
            update_warp(id, action.u.warp);
            break;
        }
        case NPCAction::WAIT: {
            if (action.u.wait.duration <= 0.0f) {
                pop_schedule(id);
            }
            action.u.wait.duration -= dds::ms_per_frame;
            break;
        }
        case NPCAction::WAIT_UNTIL: {
            if (action.u.wait_until.deadline >= m_game_state.current_time()) {
                pop_schedule(id);
            }
            break;
        }
        case NPCAction::ATTACK: {
            update_attack(id, action.u.attack);
            break;
        }
        default: {}
    }
}

void NPCDB::update_attack(NPCID id, NPCAction::U::Attack & data) {
    // TODO: don't hardcode this limit
    if (data.timer > dds::ms_per_frame * 30) {
        pop_schedule(id);
    }
    data.timer += dds::ms_per_frame;
}
