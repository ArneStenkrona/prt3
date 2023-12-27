#ifndef DDS_NPC_DB_H
#define DDS_NPC_DB_H

#include "src/daedalus/game_state/id.h"
#include "src/daedalus/game_state/game_time.h"
#include "src/daedalus/game_state/item_db.h"
#include "src/daedalus/game_state/prefab_db.h"
#include "src/daedalus/npc/npc_action.h"
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

struct NPC {
    MapPosition map_position;
    glm::vec3 direction;
    std::string model_path;
    float model_scale;
    PrefabDB::PrefabID prefab_id = PrefabDB::none;

    float collider_radius;
    float collider_height;

    float walk_force;
    float run_force;

    float friction = 10.0f / dds::time_scale; // hardcoded for now

    bool stuck = false;
    TimeMS stuck_since;

    enum Mode {
        passive, // call update when schedule is empty
        active, // call update every time npc_db updates
    };

    Mode mode = Mode::passive;
    void (*update)(NPCID, NPCDB &, prt3::Scene &);

    inline prt3::AABB get_aabb() const {
        prt3::AABB aabb;

        glm::vec3 const & pos = map_position.position;
        aabb.lower_bound = glm::vec3{
            pos.x - model_scale * collider_radius,
            pos.y,
            pos.z - model_scale * collider_radius
        };

        aabb.upper_bound = glm::vec3{
            pos.x + model_scale * collider_radius,
            pos.y + model_scale * collider_height,
            pos.z + model_scale * collider_radius
        };
        return aabb;
    }
};

struct ActionTimestamp {
    TimeMS timestamp;
    npc_action::ActionType type;
};

class NPCDB {
public:
    NPCDB(GameState & game_state);
    void on_scene_start(prt3::Scene & scene);
    void update(prt3::Scene & scene);

    void on_scene_exit();

    // Only legal if action type isn't none
    TimeMS get_action_timestamp(NPCID id) const
    { return m_current_actions[id].timestamp; }

    npc_action::ActionType get_action_type(NPCID id) const
    { return m_current_actions[id].type; }

    bool has_no_action(NPCID id) const
    { return m_current_actions[id].type == npc_action::NONE; }

    template<typename T>
    T & get_action(NPCID id)
    { return m_action_db.get_entry<T>(id); }

    template<typename T>
    T const & get_action(NPCID id) const
    { return m_action_db.get_entry<T>(id); }

    NPC & get_npc(NPCID id) { return m_npcs[id]; }
    NPC const & get_npc(NPCID id) const { return m_npcs[id]; }

    GameState & game_state() { return m_game_state; }

    MapPosition get_target_position(
        prt3::Scene const & scene,
        AnyID target
    ) const;

    float get_target_height(AnyID target) const;

    template<typename T>
    void queue_action(NPCID id, T && action) {
        npc_action::ActionUnion & au = m_new_actions[id];
        *reinterpret_cast<T*>(&au.u) = action;
        au.type = npc_action::action_to_enum<T>();
    }

    template<typename T>
    void queue_action(NPCID id, T const & action) {
        npc_action::ActionUnion & au = m_new_actions[id];
        *reinterpret_cast<T*>(&au.u) = action;
        au.type = npc_action::action_to_enum<T>();
    }

    void queue_clear_action(NPCID id) {
        npc_action::ActionUnion & au = m_new_actions[id];
        au.type = npc_action::NONE;
    }

    void set_stuck(NPCID id);

    inline bool is_loaded(NPCID id) const
    { return m_loaded_npcs.find(id) != m_loaded_npcs.end(); }

private:
    std::vector<NPC> m_npcs;
    std::unordered_map<NPCID, prt3::NodeID> m_loaded_npcs;

    ActionDatabase m_action_db;
    std::vector<ActionTimestamp> m_current_actions;
    std::unordered_map<NPCID, npc_action::ActionUnion> m_new_actions;

    GameState & m_game_state;

    NPCID push_npc();

    void load_npc(prt3::Scene & scene, NPCID id);
    void load_npcs(prt3::Scene & scene);

    template<typename T>
    inline void add_action_through_union(
        NPCID id,
        npc_action::ActionUnion const & au
    ) {
        m_action_db.add_entry<T>(id,  *reinterpret_cast<T*>(&au));
    }

    template<size_t I = 0>
    inline void update_actions(
        prt3::Scene & scene,
        std::vector<NPCID> & remove_list
    ) {
        auto & table = std::get<I>(m_action_db.get_tables());

        std::remove_reference<decltype(table)>::type::data_type::update(
            scene,
            *this,
            table.get_entries(),
            table.index_map(),
            table.num_entries(),
            remove_list
        );

        for (NPCID id : remove_list) {
            table.remove_entry(id);
            m_current_actions[id].type = npc_action::NONE;
        }
        remove_list.clear();

        if constexpr(I+1 != ActionDatabase::n_data_types)
            update_actions<I+1>(scene, remove_list);
    }

    void update_action_queues();

    void load_unload_npcs(prt3::Scene & scene);
    void move_npcs_between_rooms();
};

} // namespace dds

#endif // DDS_NPC_DB_H
