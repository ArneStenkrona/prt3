#include "npc_db.h"

#include "src/daedalus/game_state/game_state.h"
#include "src/daedalus/npc/npc_controller.h"

using namespace dds;

void on_empty_schedule_test(
    NPCID id,
    NPCDB & npc_db,
    prt3::Scene & scene
) {
    NPC & npc = npc_db.get_npc(id);

    GameState & game_state = npc_db.game_state();

    prt3::Node const & player = scene.get_node(game_state.player_id());
    MapPosition dest;
    dest.position = player.get_global_transform(scene).position;
    dest.room = game_state.current_room();

    NPCAction action;

    TimeMS time = game_state.current_time();
    if ((npc.stuck && time - npc.stuck_since > 1000 * dds::time_scale)) {
        action.type = NPCAction::WARP;
        action.u.warp.fade_time = 1000 * dds::time_scale;
        action.u.warp.timer = 0;
        action.u.warp.destination = dest;
        action.u.warp.phase = NPCAction::U::Warp::Phase::fade_in;
    } else {
        action.type = NPCAction::GO_TO_DESTINATION;
        action.u.go_to_dest.origin = npc.map_position;
        action.u.go_to_dest.destination = dest;
        action.u.go_to_dest.path_id = NO_MAP_PATH;
        action.u.go_to_dest.running = true;
    }


    npc_db.push_schedule(id, action);
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

    npc.on_empty_schedule = on_empty_schedule_test;

    NPCAction action;
    action.type = NPCAction::WAIT;
    action.u.wait.duration = 1000 * dds::time_scale;
    push_schedule(npc_id, action);
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
        data.origin = npc.map_position;
        data.path_id = map.query_map_path(data.origin, data.destination);
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
    if (schedule_empty(id)) {
        m_npcs[id].on_empty_schedule(id, *this, scene);
    }

    NPCAction & action = peek_schedule(id);

    switch (action.type) {
        case NPCAction::GO_TO_DESTINATION: {
            update_go_to_dest(id, action.u.go_to_dest);
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
        default: {}
    }

    if (schedule_empty(id)) {
        m_npcs[id].on_empty_schedule(id, *this, scene);
    }
}
