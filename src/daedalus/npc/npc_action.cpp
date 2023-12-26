#include "npc_action.h"

#include "src/daedalus/game_state/game_state.h"
#include "src/daedalus/npc/npc_db.h"

using namespace dds;

void dds::npc_action::GoToDest::update(
    prt3::Scene & /*scene*/,
    NPCDB & db,
    GoToDest * entries,
    NPCID const * index_to_id,
    size_t n_entries,
    std::vector<NPCID> & remove_list
) {
    for (uint32_t index = 0; index < n_entries; ++index) {
        auto & entry = entries[index];
        NPCID id = index_to_id[index];
        NPC & npc = db.get_npc(id);
        Map & map = db.game_state().map();

        if (!map.has_map_path(entry.path_id)) {
            entry.path_id = map.query_map_path(
                npc.map_position.position,
                entry.destination
            );
        }

        if (entry.path_id == NO_MAP_PATH) {
            db.set_stuck(id);
            remove_list.push_back(id);
            continue;
        }

        float force = entry.running ? npc.run_force : npc.walk_force;
        float velocity = (force / npc.friction);
        float length = velocity * dds::frame_dt_over_time_scale;

        bool arrived = map.advance_map_path(
            entry.path_id,
            npc.map_position.position,
            length,
            npc.map_position.position,
            npc.direction
        );

        if (arrived) {
            remove_list.push_back(id);
        }
    }
}

void dds::npc_action::Follow::update(
    prt3::Scene & scene,
    NPCDB & db,
    Follow * entries,
    NPCID const * index_to_id,
    size_t n_entries,
    std::vector<NPCID> & remove_list
) {
    for (uint32_t index = 0; index < n_entries; ++index) {
        auto & entry = entries[index];
        NPCID id = index_to_id[index];
        NPC & npc = db.get_npc(id);
        Map & map = db.game_state().map();

        MapPosition target_pos = db.get_target_position(scene, entry.target);
        float dist = glm::distance(
            npc.map_position.position,
            target_pos.position
        );

        bool has_path = map.has_map_path(entry.path_id);
        bool gen_path = !has_path;
        if (has_path) {
            float dest_dist = glm::distance(
                map.get_map_destination(entry.path_id),
                target_pos.position
            );
            gen_path |= dest_dist > entry.path_threshold;
        }

        if (gen_path) {
            entry.path_id = map.query_map_path(
                npc.map_position.position,
                target_pos.position
            );
        }

        if (entry.path_id == NO_MAP_PATH) {
            db.set_stuck(id);
            remove_list.push_back(id);
            continue;
        }

        if (dist > entry.target_dist) {
            float force = entry.running ? npc.run_force : npc.walk_force;
            float velocity = (force / npc.friction);
            float length = velocity * dds::frame_dt_over_time_scale;

            bool arrived = map.advance_map_path(
                entry.path_id,
                npc.map_position.position,
                length,
                npc.map_position.position,
                npc.direction
            );

            if (arrived && entry.stop_on_arrival) {
                remove_list.push_back(id);
            }
        }
    }
}

void dds::npc_action::Warp::update(
    prt3::Scene & /*scene*/,
    NPCDB & db,
    Warp * entries,
    NPCID const * index_to_id,
    size_t n_entries,
    std::vector<NPCID> & remove_list
) {
    for (uint32_t index = 0; index < n_entries; ++index) {
        auto & entry = entries[index];
        NPCID id = index_to_id[index];
        NPC & npc = db.get_npc(id);

        bool complete = false;

        switch (entry.phase) {
            case Phase::fade_in: {
                if (entry.timer >= entry.fade_time) {
                    entry.phase = Phase::fade_out;
                    entry.timer = 0;
                    npc.map_position = entry.destination;
                }
                break;
            }
            case Phase::fade_out: {
                if (entry.timer >= entry.fade_time) {
                    complete = true;
                }
                break;
            }
        }

        entry.timer += dds::ms_per_frame;

        if (complete) {
            remove_list.push_back(id);
        }
    }
}

void dds::npc_action::Wait::update(
    prt3::Scene & /*scene*/,
    NPCDB & /*db*/,
    Wait * entries,
    NPCID const * index_to_id,
    size_t n_entries,
    std::vector<NPCID> & remove_list
) {
    for (uint32_t index = 0; index < n_entries; ++index) {
        auto & entry = entries[index];
        NPCID id = index_to_id[index];

        if (entry.duration <= 0.0f) {
            remove_list.push_back(id);
        }
        entry.duration -= dds::ms_per_frame;
    }
}

void dds::npc_action::WaitUntil::update(
    prt3::Scene & /*scene*/,
    NPCDB & db,
    WaitUntil * entries,
    NPCID const * index_to_id,
    size_t n_entries,
    std::vector<NPCID> & remove_list
) {
    for (uint32_t index = 0; index < n_entries; ++index) {
        auto & entry = entries[index];
        NPCID id = index_to_id[index];

        if (entry.deadline >= db.game_state().current_time()) {
            remove_list.push_back(id);
        }
    }
}

void dds::npc_action::UseItem::update(
    prt3::Scene & scene,
    NPCDB & db,
    UseItem * entries,
    NPCID const * index_to_id,
    size_t n_entries,
    std::vector<NPCID> & remove_list
) {
    for (uint32_t index = 0; index < n_entries; ++index) {
        auto & entry = entries[index];
        NPCID id = index_to_id[index];
        NPC & npc = db.get_npc(id);

        /* If npc is loaded we let the npc controller handle this */
        if (npc.map_position.room != db.game_state().current_room()) {
            Item const & item = db.game_state().item_db().get(entry.item);
            if (db.get_action_timestamp(id) + item.use_duration <
                db.game_state().current_time()) {

                db.game_state().item_db().use(
                    id,
                    entry.item,
                    db.get_target_position(scene, entry.target)
                );
                remove_list.push_back(id);
            }
        }
    }
}

void dds::npc_action::Attack::update(
    prt3::Scene & /*scene*/,
    NPCDB & /*db*/,
    Attack * entries,
    NPCID const * index_to_id,
    size_t n_entries,
    std::vector<NPCID> & remove_list
) {
    for (uint32_t index = 0; index < n_entries; ++index) {
        auto & entry = entries[index];
        NPCID id = index_to_id[index];

        // TODO: don't hardcode this limit
        if (entry.timer > dds::ms_per_frame * 30) {
            remove_list.push_back(id);
        }
        entry.timer += dds::ms_per_frame;
    }
}

char const * dds::npc_action::action_type_to_str(npc_action::ActionType type) {
    switch (type) {
        case ActionType::GO_TO_DESTINATION: return "GO_TO_DESTINATION";
        case ActionType::FOLLOW: return "FOLLOW";
        case ActionType::WARP: return "WARP";
        case ActionType::WAIT: return "WAIT";
        case ActionType::WAIT_UNTIL: return "WAIT_UNTIL";
        case ActionType::USE_ITEM: return "USE_ITEM";
        case ActionType::ATTACK: return "ATTACK";
        case ActionType::NONE: return "NONE";
    }
}
