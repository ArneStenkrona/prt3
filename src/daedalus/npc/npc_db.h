#ifndef DDS_NPC_DB_H
#define DDS_NPC_DB_H

#include "src/daedalus/game_state/game_time.h"
#include "src/daedalus/game_state/prefab_db.h"
#include "src/daedalus/map/map.h"
#include "src/engine/scene/scene.h"
#include "src/engine/scene/node.h"

#include <vector>
#include <queue>
#include <unordered_map>
#include <string>

namespace dds {

class GameState;
class NPCDB;

typedef uint32_t NPCID;

struct NPC {
    MapPosition map_position;
    glm::vec3 direction;
    std::string model_path;
    glm::vec3 model_scale;
    PrefabDB::PrefabID prefab_id = PrefabDB::none;

    float collider_radius;
    float collider_length;

    float walk_force;
    float run_force;

    float friction = 10.0f / dds::time_scale; // hardcoded for now

    bool stuck = false;
    TimeMS stuck_since;

    void (*on_empty_schedule)(NPCID, NPCDB &, prt3::Scene &);
};

struct NPCAction {
    enum ActionType {
        GO_TO_DESTINATION,
        WARP,
        WAIT,
        WAIT_UNTIL,
        NONE
    };

    ActionType type;
    union U {
        constexpr U() : go_to_dest{} {}
        struct GoToDest {
            MapPosition origin;
            MapPosition destination;
            MapPathID path_id;
            bool running;
        } go_to_dest;

        struct Warp {
            enum class Phase {
                fade_out,
                fade_in
            };

            TimeMS fade_time;
            TimeMS timer;
            Phase phase;
            MapPosition destination;
        } warp;

        struct Wait {
            TimeMS duration;
        } wait;

        struct WaitUntil {
            TimeMS deadline;
        } wait_until;
    } u;
};

typedef std::queue<NPCAction> NPCSchedule;

enum class ScheduleStatus {
    success,
    // go to dest
    no_path_found,
    stuck_on_path
};

class NPCDB {
public:
    NPCDB(GameState & game_state);
    void on_scene_start(prt3::Scene & scene);
    void update(prt3::Scene & scene);

    void on_scene_exit();

    bool schedule_empty(NPCID id) const { return m_schedules[id].empty(); }
    NPCAction & peek_schedule(NPCID id) { return m_schedules[id].front(); }
    void pop_schedule(
        NPCID id,
        ScheduleStatus status = ScheduleStatus::success
    );
    void push_schedule(NPCID id, NPCAction const & action)
    { return m_schedules[id].push(action); }

    NPC & get_npc(NPCID id) { return m_npcs[id]; }

    GameState & game_state() { return m_game_state; }

private:
    std::vector<NPC> m_npcs;
    std::vector<NPCSchedule> m_schedules;
    std::unordered_map<NPCID, prt3::NodeID> m_loaded_npcs;

    GameState & m_game_state;

    NPCID push_npc();

    void load_npc(prt3::Scene & scene, NPCID id);
    void load_npcs(prt3::Scene & scene);

    void update_npc(NPCID id, prt3::Scene & scene);

    void update_go_to_dest(NPCID id, NPCAction::U::GoToDest & data);

    void update_warp(NPCID id, NPCAction::U::Warp & data);
};

} // namespace dds

#endif // DDS_NPC_DB_H
