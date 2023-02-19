#ifndef PRT3_GAME_STATE_H
#define PRT3_GAME_STATE_H

#include "src/engine/component/script/script.h"
#include "src/engine/component/script/camera_controller.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/door.h"
#include "src/util/serialization_util.h"
#include "src/engine/scene/scene_manager.h"
#include "src/engine/scene/prefab.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>
#include <cmath>

namespace prt3 {

class GameState : public Script {
public:
    explicit GameState(Scene & scene, NodeID node_id)
        : Script(scene, node_id) {}

    explicit GameState(std::istream &, Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    virtual void on_start(Scene & scene) {
        glm::vec3 spawn_position;
        glm::vec3 dir = glm::vec3{0.0f, 0.0f, 1.0f};
        for (Door const & door : scene.get_all_components<Door>()) {
            if (door.id() == m_entry_door_id) {
                Node const & door_node = scene.get_node(door.node_id());
                glm::vec3 door_position =
                    door_node.get_global_transform(scene).position;
                spawn_position = door_position + door.entry_offset();

                if (door.entry_offset().x != 0.0f ||
                    door.entry_offset().z != 0.0f) {
                    dir = glm::normalize(glm::vec3{
                        door.entry_offset().x,
                        0.0f,
                        door.entry_offset().z
                    });
                }
                break;
            }
        }
        NodeID id = m_player_prefab.instantiate(scene, scene.get_root_id());
        Node & node = scene.get_node(id);
        node.set_global_position(scene, spawn_position);

        float yaw = std::atan2(dir.x, dir.z);

        glm::quat rot = glm::quat_cast(
            glm::eulerAngleYXZ(yaw, 0.0f, 0.0f)
        );

        node.local_transform().rotation = node.local_transform().rotation * rot;

        glm::vec3 camera_dir = glm::normalize(
            glm::vec3{0.0f, 0.5f, 0.0f} - dir
        );

        NodeID cam_id = m_camera_prefab.instantiate(scene, scene.get_root_id());
        CameraController & cam = *scene.get_component<ScriptSet>(cam_id)
            .get_script<CameraController>(scene);
        cam.set_direction(camera_dir);
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
