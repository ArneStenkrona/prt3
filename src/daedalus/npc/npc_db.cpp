#include "npc_db.h"

#include "src/daedalus/game_state/game_state.h"
#include "src/daedalus/npc/npc_controller.h"

#include "src/util/geometry_util.h"

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
        float arcane_threshold = 12.5f;
        float melee_threshold = 3.5f;

        if (dist < arcane_threshold &&
            npc_db.game_state().item_db().ready_to_use(
                id, ItemID::item_spell_flame_pillar
        )) {
            npc_action::UseItem use_item;
            use_item.item = ItemID::item_spell_flame_pillar;
            use_item.target.type = IDType::dds_id_type_player;
            use_item.activated = false;
            npc_db.queue_action<npc_action::UseItem>(id, std::move(use_item));
        } else if (dist < arcane_threshold &&
            npc_db.game_state().item_db().ready_to_use(
                id, ItemID::item_spell_fire_rock
        )) {
            npc_action::UseItem use_item;
            use_item.item = ItemID::item_spell_fire_rock;
            use_item.target.type = IDType::dds_id_type_player;
            use_item.activated = false;
            npc_db.queue_action<npc_action::UseItem>(id, std::move(use_item));
        } else if (dist < melee_threshold) {
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

NPCDB::NPCDB(GameState & game_state)
 : m_game_state{game_state} {
    /* for testing */
    NPCID npc_id = push_npc();

    NPC & npc = m_npcs[npc_id];
    npc.map_position.room = 4;
    npc.map_position.position = glm::vec3{-4.0f, 0.5f, -0.5f};
    npc.model_path = "assets/models/boss1/boss1.fbx";
    npc.model_scale = 0.62f;
    npc.prefab_id = PrefabDB::dark_flames;
    npc.collider_radius = 1.0f;
    npc.collider_height = 3.75f;
    npc.walk_force = 50.0f / dds::time_scale;
    npc.run_force = 118.0f / dds::time_scale;

    npc.mode = NPC::Mode::active;
    npc.update = npc_update_test;
}

void NPCDB::on_scene_start(prt3::Scene & scene) {
    load_npcs(scene);
}

void NPCDB::update(prt3::Scene & scene) {
    thread_local std::vector<NPCID> remove_list;
    update_actions(scene, remove_list);
    update_action_queues();
    load_unload_npcs(scene);
}

void NPCDB::load_unload_npcs(prt3::Scene & scene) {
    move_npcs_between_rooms();

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
        if (has_no_action(id) || m_npcs[id].mode == NPC::Mode::active) {
            m_npcs[id].update(id, *this, scene);
        }
    }
}

void NPCDB::update_action_queues() {
    for (auto & pair : m_new_actions) {
        NPCID id = pair.first;
        if (m_current_actions[id].type != npc_action::NONE) {
            m_action_db.remove_entry_by_table_index(
                m_current_actions[id].type,
                id
            );
            m_current_actions[id].type = npc_action::NONE;
        }

        npc_action::ActionUnion & au = pair.second;
        if (au.type == npc_action::NONE) {
            continue;
        }

        void * action_p = reinterpret_cast<void*>(&au.u);
        m_action_db.add_entry_by_table_index(au.type, id, action_p);
        m_current_actions[id].timestamp = m_game_state.current_time();
        m_current_actions[id].type = au.type;
        /* clear status when new action is set */
        if (au.type != npc_action::GO_TO_DESTINATION &&
            au.type != npc_action::FOLLOW) {
            m_npcs[id].stuck = false;
        }
    }
    m_new_actions.clear();
}

void NPCDB::on_scene_exit() {
    m_loaded_npcs.clear();
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
            return m_game_state.object_db().get_object(target.id).position;
        }
        case IDType::dds_id_type_item: {
            // nonsensical
            MapPosition res;
            res.position = glm::vec3{std::numeric_limits<float>::infinity()};
            return res;
        }
    }
}

float NPCDB::get_target_height(AnyID target) const {
    switch (target.type) {
        case IDType::dds_id_type_player: {
            return 3.0f; // hardcoded for now
        }
        case IDType::dds_id_type_npc: {
            return m_npcs[target.id].model_scale *
                   m_npcs[target.id].collider_height;
        }
        case IDType::dds_id_type_object: {
            return 0.0f;
        }
        case IDType::dds_id_type_item: {
            // nonsensical
            return 0.0f;
        }
    }
}


NPCID NPCDB::push_npc() {
    NPCID id = m_npcs.size();
    m_npcs.push_back({});
    m_current_actions.push_back({0, npc_action::NONE});
    return id;
}

void NPCDB::load_npc(prt3::Scene & scene, NPCID id) {
    NPC const & npc = m_npcs[id];

    prt3::NodeID node_id = scene.add_node_to_root("NPC");
    scene.add_model_to_scene_from_path(npc.model_path, node_id, true, true);

    prt3::Node & node = scene.get_node(node_id);
    node.set_global_position(scene, npc.map_position.position);
    node.set_global_scale(scene, glm::vec3{npc.model_scale});

    prt3::Capsule capsule{};
    capsule.radius = npc.collider_radius;
    capsule.start.y = npc.collider_radius;
    capsule.end.y = npc.collider_radius + npc.collider_height;
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

void NPCDB::set_stuck(NPCID id) {
    if (!m_npcs[id].stuck) {
        m_npcs[id].stuck_since = m_game_state.current_time();
    }
    m_npcs[id].stuck = true;
}

void NPCDB::move_npcs_between_rooms() {
    thread_local std::vector<prt3::ColliderID> ids;
    ids.clear();

    Map & map = m_game_state.map();

    struct IntersectRes {
        NPCID npc_id;
        int32_t num;
    };

    thread_local std::vector<IntersectRes> res;
    res.clear();

    for (NPCID i = 0; i < m_npcs.size(); ++i) {
        NPC & npc = m_npcs[i];
        RoomID room_id = npc.map_position.room;

        prt3::AABB aabb = npc.get_aabb();

        size_t before = ids.size();
        map.query_doors(room_id, aabb, ids);

        if (ids.size() > before) {
            int32_t n_ids = static_cast<int32_t>(ids.size() - before);
            res.push_back({i, n_ids});
        }
    }

    if (ids.empty()) {
        return;
    }

    std::vector<glm::vec3> const & geom = map.door_geometry();

    uint32_t id_index = 0;
    for (IntersectRes & r : res) {
        NPC & npc = m_npcs[r.npc_id];
        prt3::AABB aabb = npc.get_aabb();
        glm::vec3 c = 0.5f * (aabb.lower_bound + aabb.upper_bound); // center
        glm::vec3 hs = 0.5f * (aabb.upper_bound - aabb.lower_bound); // halfsize

        uint32_t n_ids = r.num;
        r.num = -1;
        for (uint32_t i = 0; i < n_ids; ++i) {
            uint32_t door_id = ids[id_index + i];
            glm::vec3 const * gs = &geom[map.door_to_vertex_index(door_id)];
            if (prt3::triangle_box_overlap(c, hs, gs[0], gs[1], gs[3]) ||
                prt3::triangle_box_overlap(c, hs, gs[2], gs[1], gs[3])) {
                r.num = door_id;
                break;
            }
        }

        id_index += n_ids;
    }

    for (IntersectRes & r : res) {
        if (r.num == -1) continue;
        NPC & npc = m_npcs[r.npc_id];
        glm::vec3 p_door = m_game_state.get_door_local_position(
            r.num,
            npc.map_position.position
        );
        uint32_t dest_door = map.get_door_destination_id(r.num);
        glm::vec3 new_pos = p_door + map.get_door_entry_position(dest_door);
        npc.map_position.position = new_pos;
        npc.map_position.room = map.door_to_room(dest_door);
    }
}
