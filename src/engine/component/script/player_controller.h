#ifndef PRT3_PLAYER_CONTROLLER_H
#define PRT3_PLAYER_CONTROLLER_H

#include "src/engine/component/script/character_controller.h"
#include "src/engine/scene/scene_manager.h"

namespace prt3 {

class PlayerController : public CharacterController {
public:
    explicit PlayerController(Scene & scene, NodeID m_node_id)
        : CharacterController(scene, m_node_id) {}

    explicit PlayerController(std::istream &, Scene & scene, NodeID m_node_id)
        : CharacterController(scene, m_node_id) {}

    void exclude_player_from_scene_fade(Scene & scene);

    virtual void on_init(Scene & scene);

    virtual void update_input(Scene & scene, float /*delta_time*/);
    virtual void on_update(Scene & scene, float delta_time);
private:
    NodeID m_blob_shadow_id;

REGISTER_SCRIPT_BEGIN(PlayerController, player_controller, 3968611710651155566)
REGISTER_SERIALIZED_FIELD(m_walk_force)
REGISTER_SERIALIZED_FIELD(m_run_force)
REGISTER_SCRIPT_END()
};

} // namespace prt3

#endif // PRT3_PLAYER_CONTROLLER_H
