#ifndef DDS_NPC_ACTION_H
#define DDS_NPC_ACTION_H

#include "src/daedalus/game_state/game_time.h"
#include "src/daedalus/game_state/item_db.h"
#include "src/daedalus/game_state/id.h"
#include "src/daedalus/map/map.h"
#include "src/util/database/database.h"

namespace dds {

class NPCDB;

namespace npc_action {

struct GoToDest {
    glm::vec3 destination;
    MapPathID path_id;
    bool running;
    static void update(
        prt3::Scene & scene,
        NPCDB & db,
        GoToDest * entries,
        NPCID const * index_to_id,
        size_t n_entries,
        std::vector<NPCID> & remove_list
    );
};

struct Follow {
    AnyID target;
    MapPathID path_id;
    float path_threshold;
    float target_dist;
    bool stop_on_arrival;
    bool running;
    static void update(
        prt3::Scene & scene,
        NPCDB & db,
        Follow * entries,
        NPCID const * index_to_id,
        size_t n_entries,
        std::vector<NPCID> & remove_list
    );
};

struct Warp {
    enum class Phase {
        fade_out,
        fade_in
    };

    TimeMS fade_time;
    TimeMS timer;
    Phase phase;
    MapPosition destination;

    static void update(
        prt3::Scene & scene,
        NPCDB & db,
        Warp * entries,
        NPCID const * index_to_id,
        size_t n_entries,
        std::vector<NPCID> & remove_list
    );
};

struct Wait {
    TimeMS duration;
    static void update(
        prt3::Scene & scene,
        NPCDB & db,
        Wait * entries,
        NPCID const * index_to_id,
        size_t n_entries,
        std::vector<NPCID> & remove_list
    );
};

struct WaitUntil {
    TimeMS deadline;
    static void update(
        prt3::Scene & scene,
        NPCDB & db,
        WaitUntil * entries,
        NPCID const * index_to_id,
        size_t n_entries,
        std::vector<NPCID> & remove_list
    );
};

struct UseItem {
    ItemID item;
    AnyID target;
    bool activated;
    static void update(
        prt3::Scene & scene,
        NPCDB & db,
        UseItem * entries,
        NPCID const * index_to_id,
        size_t n_entries,
        std::vector<NPCID> & remove_list
    );
};

struct Attack {
    AnyID target;
    TimeMS timer;
    bool activated;
    static void update(
        prt3::Scene & scene,
        NPCDB & db,
        Attack * entries,
        NPCID const * index_to_id,
        size_t n_entries,
        std::vector<NPCID> & remove_list
    );
};

} // namespace npc_action

using ActionDatabase = prt3::Database<
    /* id type */
    NPCID,
    /* data types */
    npc_action::GoToDest,
    npc_action::Follow,
    npc_action::Warp,
    npc_action::Wait,
    npc_action::WaitUntil,
    npc_action::UseItem,
    npc_action::Attack
>;

namespace npc_action {

enum ActionType {
    GO_TO_DESTINATION = ActionDatabase::get_table_index<GoToDest>(),
    FOLLOW = ActionDatabase::get_table_index<Follow>(),
    WARP = ActionDatabase::get_table_index<Warp>(),
    WAIT = ActionDatabase::get_table_index<Wait>(),
    WAIT_UNTIL = ActionDatabase::get_table_index<WaitUntil>(),
    USE_ITEM = ActionDatabase::get_table_index<UseItem>(),
    ATTACK = ActionDatabase::get_table_index<Attack>(),
    NONE
};

char const * action_type_to_str(ActionType type);

template<typename T>
inline ActionType action_to_enum() {
    return static_cast<ActionType>(ActionDatabase::get_table_index<T>());
}

struct ActionUnion {
    union U {
        U() : go_to_dest{} {}
        npc_action::GoToDest go_to_dest;
        npc_action::Follow follow;
        npc_action::Warp warp;
        npc_action::Wait wait;
        npc_action::WaitUntil wait_until;
        npc_action::UseItem use_item;
        npc_action::Attack attack;
    } u;
    ActionType type;
};

} // namespace npc_action

} // namespace dds;

#endif // DDS_NPC_ACTION_H
