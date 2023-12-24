#ifndef DDS_NPC_CONTROLLER_H
#define DDS_NPC_CONTROLLER_H

#include "src/daedalus/character/character_controller.h"
#include "src/daedalus/game_state/game_state.h"
#include "src/engine/scene/scene.h"

#include <vector>

namespace dds {

class NPCController : public CharacterController {
public:
    explicit NPCController(prt3::Scene & scene, prt3::NodeID node_id)
    : CharacterController(scene, node_id) {}

    explicit NPCController(
        prt3::Scene & scene,
        prt3::NodeID node_id,
        NPCID npc_id
    ) : CharacterController(scene, node_id), m_npc_id{npc_id} {}

    explicit NPCController(
        std::istream &,
        prt3::Scene & scene,
        prt3::NodeID node_id
    ) : CharacterController(scene, node_id) {}

    virtual void on_init(prt3::Scene & scene);

    virtual void on_update(prt3::Scene & scene, float delta_time);

    void set_npc_id(NPCID id) { m_npc_id = id; }
private:
    NPCID m_npc_id;
    GameState * m_game_state;

    static constexpr unsigned int m_rolling_avg_n = 10;
    float m_movement_performance = 1.0f;

    void update_moving(prt3::Scene & scene, float delta_time);
    void update_warp(prt3::Scene & scene, float delta_time);
    void update_attack(prt3::Scene & scene, float delta_time);
    void update_use_item(prt3::Scene & scene, float delta_time);

    virtual void update_input(prt3::Scene & /*scene*/, float /*delta_time*/);

REGISTER_SCRIPT(NPCController, npc_controller, 15129599306800160206)
};

} // namespace dds

#endif // DDS_NPC_CONTROLLER_H
