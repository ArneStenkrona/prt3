#ifndef PRT3_GAME_STATE_H
#define PRT3_GAME_STATE_H

#include "src/engine/component/script/script.h"
#include "src/engine/component/script/player_controller.h"
#include "src/engine/component/script/camera_controller.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/door.h"
#include "src/util/serialization_util.h"
#include "src/engine/scene/scene_manager.h"
#include "src/engine/audio/audio_manager.h"
#include "src/engine/scene/prefab.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cmath>
#include <array>
#include <unordered_set>

namespace prt3 {

class GameState : public Script {
public:
    explicit GameState(Scene & scene, NodeID node_id)
        : Script(scene, node_id) {
            init_resources(scene);
        }

    explicit GameState(std::istream &, Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {
            init_resources(scene);
        }

    virtual void on_signal(
        Scene & scene,
        SignalString const & signal,
        void * data
    ) {
        if (signal == "__scene_transition_out__") {
            float t = reinterpret_cast<float*>(data)[0];
            float dt = reinterpret_cast<float*>(data)[1];

            for (Door const & door : scene.get_all_components<Door>()) {
                if (door.id() == m_entry_door_id) {

                    Camera & cam = scene.get_camera();
                    Node & player = scene.get_node(m_player_id);

                    glm::vec3 translation = -t * dt * door.entry_offset();
                    cam.transform().position += translation;
                    player.translate_node(scene, translation);
                }
            }
        } else if (signal == "__scene_exit__") {
            PlayerController * controller =
                scene.get_script_from_node<PlayerController>(m_player_id);
            m_player_state = controller->serialize_state(scene);

            CameraController & cam = *scene.get_component<ScriptSet>(m_camera_id)
                .get_script<CameraController>(scene);

            m_cam_yaw = cam.yaw();
            m_cam_pitch = cam.pitch();

            for (Door const & door : scene.get_all_components<Door>()) {
                if (door.id() == m_exit_door_id) {
                    Node const & door_node = scene.get_node(door.node_id());
                    Transform door_tform =
                        door_node.get_global_transform(scene);
                    glm::vec3 door_position = door_tform.position;

                    Node & player = scene.get_node(m_player_id);
                    glm::vec3 v = player.get_global_transform(scene).position -
                                    door_position;

                    glm::vec3 n = door.entry_offset();
                    if (n != glm::vec3{0.0f}) n = glm::normalize(n);
                    m_player_door_offset = v - glm::dot(v, n) * n;
                    break;
                }
            }
        }
    }

    virtual void on_start(Scene & scene) {
        scene.connect_signal("__scene_transition_out__", this);
        scene.connect_signal("__scene_exit__", this);

        glm::vec3 spawn_position;
        glm::vec3 dir = glm::vec3{0.0f, 0.0f, 1.0f};
        for (Door const & door : scene.get_all_components<Door>()) {
            if (door.id() == m_entry_door_id) {
                Node const & door_node = scene.get_node(door.node_id());
                glm::vec3 door_position =
                    door_node.get_global_transform(scene).position;
                spawn_position =
                    door_position +
                    door.entry_offset() +
                    m_player_door_offset;

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
        m_player_id = m_player_prefab.instantiate(scene, scene.get_root_id());
        Node & node = scene.get_node(m_player_id);
        node.set_global_position(scene, spawn_position);

        float yaw = std::atan2(dir.x, dir.z);

        glm::quat rot = glm::quat_cast(
            glm::eulerAngleYXZ(yaw, 0.0f, 0.0f)
        );

        node.set_global_rotation(scene, rot);

        PlayerController * controller =
            scene.get_script_from_node<PlayerController>(m_player_id);
        controller->deserialize_state(scene, m_player_state);

        m_camera_id = m_camera_prefab.instantiate(scene, scene.get_root_id());
        CameraController & cam = *scene.get_component<ScriptSet>(m_camera_id)
            .get_script<CameraController>(scene);

        cam.yaw() = m_cam_yaw;
        cam.pitch() = m_cam_pitch;

        // play soundtrack
        if (scene.audio_manager().get_playing_midi() != m_midi) {
            scene.audio_manager().play_midi(m_midi, m_sound_font);
        }
    }

    virtual void on_init(Scene &) {
    }

    virtual void on_update(Scene &, float) {
    }

    void set_exit_door_id(DoorID id) { m_exit_door_id = id; }
    void set_entry_door_id(DoorID id) { m_entry_door_id = id; }

    void register_door_overlap(Scene & scene, Door & door) {
        scene.scene_manager()
            .queue_scene(door.destination_scene_path().data());
        m_exit_door_id = door.id();
        m_entry_door_id = door.destination_id();
    }

private:
    DoorID m_exit_door_id = 0;
    DoorID m_entry_door_id = 0;

    Prefab m_player_prefab{"assets/prefabs/player.prefab"};
    NodeID m_player_id;
    CharacterController::SerializedState m_player_state = {};
    glm::vec3 m_player_door_offset;

    Prefab m_camera_prefab{"assets/prefabs/camera.prefab"};
    NodeID m_camera_id;

    float m_cam_yaw;
    float m_cam_pitch;

    MidiID m_midi;
    SoundFontID m_sound_font;

    void init_resources(Scene & scene) {
        // sound
        m_midi =
            scene.audio_manager().load_midi("assets/audio/tracks/enter.mid");
        m_sound_font =
            scene.audio_manager().load_sound_font("assets/audio/soundfonts/CT2MGM.sf2");
    }

REGISTER_SCRIPT(GameState, game_state, 11630114958491958378)
};

} // namespace prt3

#endif // PRT3_GAME_STATE_H
