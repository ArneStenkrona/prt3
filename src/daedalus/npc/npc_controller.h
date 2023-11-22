#ifndef DDS_NPC_CONTROLLER_H
#define DDS_NPC_CONTROLLER_H

#include "src/daedalus/game_state/game_state.h"
#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"

#include <vector>

namespace dds {

class NPCController : public prt3::Script {
public:
    explicit NPCController(prt3::Scene & scene, prt3::NodeID node_id)
        : prt3::Script(scene, node_id) {}

        explicit NPCController(
            prt3::Scene & scene,
            prt3::NodeID node_id,
            NPCID npc_id
        )
        : prt3::Script(scene, node_id), m_npc_id{npc_id} {}

    explicit NPCController(
        std::istream &,
        prt3::Scene & scene,
        prt3::NodeID node_id
    )
        : prt3::Script(scene, node_id) {}

    virtual void on_init(prt3::Scene & scene);

    virtual void on_update(prt3::Scene & scene, float delta_time);

    void set_npc_id(NPCID id) { m_npc_id = id; }
private:
    NPCID m_npc_id;
    GameState * m_game_state;

    void init_animation(prt3::Scene & scene);

    void update_action(prt3::Scene & scene, float delta_time);
    void update_go_to_dest(prt3::Scene & scene, float delta_time);

REGISTER_SCRIPT(NPCController, npc_controller, 15129599306800160206)
};

} // namespace dds

#endif // DDS_NPC_CONTROLLER_H
