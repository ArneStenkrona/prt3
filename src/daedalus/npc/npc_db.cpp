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
    action.type = NPCAction::GO_TO_DESTINATION;
    action.u.go_to_dest.origin = npc.map_position;
    action.u.go_to_dest.destination = dest;
    action.u.go_to_dest.path_id = NO_MAP_PATH;
    action.u.go_to_dest.t = 0.0f;

    npc_db.push_schedule(id, action);
}

NPCDB::NPCDB(GameState & game_state)
 : m_game_state{game_state} {
    /* for testing */
    NPCID npc_id = push_npc();

    NPC & npc = m_npcs[npc_id];
    npc.map_position.room = 4;
    npc.map_position.position = glm::vec3{-4.0f, 0.5f, -0.5f};
    npc.model_path = "assets/models/stranger/stranger.fbx";
    npc.model_scale = glm::vec3{0.5f};
    npc.collider_radius = 0.5f;
    npc.collider_length = 1.0f;
    npc.speed = 0.1f;
    npc.on_empty_schedule = on_empty_schedule_test;

    NPCAction action;
    action.type = NPCAction::WAIT;
    action.u.wait.duration = 1000 * dds::time_scale;
    push_schedule(npc_id, action);
 }

void NPCDB::update(prt3::Scene & scene) {
    for (NPCID id = 0; id < m_npcs.size(); ++id) {
        NPC & npc = m_npcs[id];
        if (m_loaded_npcs.find(id) == m_loaded_npcs.end()) {
            if (npc.map_position.room == m_game_state.current_room()) {
                load_npc(scene, id);
            }
        }
    }

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

    prt3::Capsule capsule{};
    capsule.radius = npc.collider_radius;
    capsule.start.y = npc.collider_radius;
    capsule.end.y = npc.collider_radius + npc.collider_length;
    scene.add_component<prt3::ColliderComponent>(
        node_id,
        prt3::ColliderType::collider,
        capsule
    );

    prt3::ScriptSet script_set = scene.add_component<prt3::ScriptSet>(node_id);
    prt3::ScriptID script_id =
        script_set.add_script<NPCController>(scene);
    NPCController & script =
        *dynamic_cast<NPCController*>(scene.get_script(script_id));
    script.set_npc_id(id);
    script.set_game_state(m_game_state);

    m_loaded_npcs[id] = node_id;

    prt3::Node & node = scene.get_node(node_id);
    node.set_global_position(scene, npc.map_position.position);
    node.set_global_scale(scene, npc.model_scale);
}

void NPCDB::update_go_to_dest(NPCID id, NPCAction::U::GoToDest & data) {
    NPC & npc = m_npcs[id];
    Map & map = m_game_state.map();

    if (!map.has_map_path(data.path_id)) {
        data.origin = npc.map_position;
        data.t = 0.0f;
        data.path_id = map.query_map_path(data.origin, data.destination);
    }

    if (data.path_id == NO_MAP_PATH) {
        pop_schedule(id);
        return;
    }

    float length = npc.speed;
    data.t += length / map.get_map_path_length(data.path_id);
    npc.map_position = map.interpolate_map_path(data.path_id, data.t);

    if (data.t >= 1.0f) {
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
