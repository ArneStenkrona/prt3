#ifndef DDS_PLAYER_CONTROLLER_H
#define DDS_PLAYER_CONTROLLER_H

#include "src/daedalus/character/character_controller.h"
#include "src/engine/scene/scene_manager.h"

namespace dds {

class PlayerController : public CharacterController {
public:
    explicit PlayerController(prt3::Scene & scene, prt3::NodeID m_node_id)
        : CharacterController(scene, m_node_id) {}

    explicit PlayerController(
        std::istream &,
        prt3::Scene & scene,
        prt3::NodeID m_node_id
    )
        : CharacterController(scene, m_node_id) {}

    void exclude_player_from_scene_fade(prt3::Scene & scene);

    virtual void on_init(prt3::Scene & scene);

    virtual void update_input(prt3::Scene & scene, float /*delta_time*/);
    virtual void on_update(prt3::Scene & scene, float delta_time);
private:
    prt3::NodeID m_blob_shadow_id;

REGISTER_SCRIPT_BEGIN(PlayerController, player_controller, 3968611710651155566)
REGISTER_SERIALIZED_FIELD(m_walk_force)
REGISTER_SERIALIZED_FIELD(m_run_force)
REGISTER_SCRIPT_END()
};

} // namespace dds

#endif // DDS_PLAYER_CONTROLLER_H
