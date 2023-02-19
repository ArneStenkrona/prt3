#ifndef PRT3_GAME_STATE_H
#define PRT3_GAME_STATE_H

#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/door.h"
#include "src/util/serialization_util.h"
#include "src/engine/scene/scene_manager.h"
#include "src/engine/scene/prefab.h"

#include <iostream>

namespace prt3 {

class GameState : public Script {
public:
    explicit GameState(Scene & scene, NodeID node_id)
        : Script(scene, node_id) {}

    explicit GameState(std::istream &, Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    virtual void on_start(Scene & scene) {
        glm::vec3 spawn_position;
        for (Door const & door : scene.get_all_components<Door>()) {
            if (door.id() == m_entry_door_id) {
                Node const & door_node = scene.get_node(door.node_id());
                spawn_position =
                    door_node.get_global_transform(scene).position;
                break;
            }
        }
        NodeID id = m_player_prefab.instantiate(scene, scene.get_root_id());
        Node & node = scene.get_node(id);
        node.set_global_position(scene, spawn_position);

        m_camera_prefab.instantiate(scene, scene.get_root_id());
    }

    virtual void on_init(Scene &) {
    }

    virtual void on_update(Scene &, float) {
    }

    virtual void save_state(std::ostream & out) const {
        write_stream(out, m_entry_door_id);
    }

    virtual void restore_state(std::istream & in) {
        read_stream(in, m_entry_door_id);
    }

    void set_entry_door_id(DoorID id) { m_entry_door_id = id; }
private:
    DoorID m_entry_door_id = 0;

    Prefab m_player_prefab{"assets/prefabs/player.prefab"};
    Prefab m_camera_prefab{"assets/prefabs/camera.prefab"};

REGISTER_SCRIPT(GameState, game_state, 11630114958491958378)
};

} // namespace prt3

#endif // PRT3_GAME_STATE_H
