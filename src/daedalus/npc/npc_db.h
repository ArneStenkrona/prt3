#ifndef DDS_NPC_DB_H
#define DDS_NPC_DB_H

#include "src/daedalus/game_state/game_time.h"
#include "src/daedalus/map/map.h"
#include "src/engine/scene/scene.h"
#include "src/engine/scene/node.h"

#include <vector>
#include <queue>
#include <unordered_map>
#include <string>

namespace dds {

class GameState;

typedef uint32_t NPCID;

struct NPC {
    MapPosition map_position;
    struct {
        glm::vec3 b;
        float t;
    } unloaded_position;

    std::string model_path;

    float collider_radius;
    float collider_length;

    float speed;


    void (*on_empty_schedule)(NPCID, GameState*);
};

struct NPCAction {
    enum ActionType {
        GO_TO_DESTINATION,
        WAIT,
        WAIT_UNTIL,
        NONE
    };

    ActionType type;
    union U {
        struct GoToDest {
            MapPosition origin;
            MapPosition destination;
            MapPathID path_id;
            float t;
        } go_to_dest;

        struct Wait {
            TimeMS duration;
        } wait;

        struct WaitUntil {
            TimeMS deadline;
        } wait_until;
    } u;
};

typedef std::queue<NPCAction> NPCSchedule;

class NPCDB {
public:
    NPCDB(GameState & game_state);
    void update(prt3::Scene & scene);

    bool schedule_empty(NPCID id) const { return m_schedules[id].empty(); }
    NPCAction & peek_schedule(NPCID id) { return m_schedules[id].front(); }
    void pop_schedule(NPCID id) { return m_schedules[id].pop(); }
    void push_schedule(NPCID id, NPCAction const & action)
    { return m_schedules[id].push(action); }

    NPC & get_npc(NPCID id) { return m_npcs[id]; }

private:
    std::vector<NPC> m_npcs;
    std::vector<NPCSchedule> m_schedules;
    std::unordered_map<NPCID, prt3::NodeID> m_loaded_npcs;

    RoomID m_current_room;

    void load_NPC(prt3::Scene & scene, NPCID id);
    void update_npc(NPCID id);
    void update_go_to_dest(NPCID id, NPCAction::U::GoToDest & data);

    GameState & m_game_state;
};

} // namespace dds

#endif // DDS_NPC_DB_H
