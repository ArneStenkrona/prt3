#include "game_state.h"

using namespace dds;

#define MAP_PATH "assets/models/map/island.map" // hard-coded for now

GameState::GameState(prt3::Scene & scene, prt3::NodeID node_id)
 : Script(scene, node_id),
   m_map{MAP_PATH},
   m_npc_db{*this},
   m_game_gui{scene} {
    init_resources(scene);
}

GameState::GameState(
    std::istream &,
    prt3::Scene & scene,
    prt3::NodeID m_node_id
)
 : prt3::Script(scene, m_node_id),
   m_map{MAP_PATH},
   m_npc_db{*this},
   m_game_gui{scene} {
    init_resources(scene);
}

void GameState::on_signal(
    prt3::Scene & scene,
    prt3::SignalString const & signal,
    void * data
) {
    if (signal == "__scene_transition_out__") {
        float t = reinterpret_cast<float*>(data)[0];
        float dt = reinterpret_cast<float*>(data)[1];

        for (prt3::Door const & door : scene.get_all_components<prt3::Door>()) {
            if (door.id() == m_entry_door_id) {

                prt3::Camera & cam = scene.get_camera();
                prt3::Node & player = scene.get_node(m_player_id);

                glm::vec3 translation = -t * dt * door.entry_offset();
                cam.transform().position += translation;
                player.translate_node(scene, translation);
            }
        }
    } else if (signal == "__scene_exit__") {
        prt3::PlayerController * controller =
            scene.get_script_from_node<prt3::PlayerController>(m_player_id);
        m_player_state = controller->serialize_state(scene);

        prt3::CameraController & cam =
            *scene.get_component<prt3::ScriptSet>(m_camera_id)
                .get_script<prt3::CameraController>(scene);

        m_cam_yaw = cam.yaw();
        m_cam_pitch = cam.pitch();

        for (prt3::Door const & door : scene.get_all_components<prt3::Door>()) {
            if (door.id() == m_exit_door_id) {
                prt3::Node const & door_node = scene.get_node(door.node_id());
                prt3::Transform door_tform =
                    door_node.get_global_transform(scene);
                glm::vec3 door_position = door_tform.position;
                glm::vec3 entry_position = door_position + door.entry_offset();

                prt3::Node & player = scene.get_node(m_player_id);
                glm::vec3 player_pos = player.get_global_transform(scene).position;
                glm::vec3 v = player_pos - entry_position;

                glm::vec3 n = door_tform.get_up();
                if (n != glm::vec3{0.0f}) n = glm::normalize(n);
                m_player_door_offset = v - glm::dot(v, n) * n;
                break;
            }
        }

        m_npc_db.on_scene_exit();
    }
}

void GameState::on_start(prt3::Scene & scene) {
    scene.connect_signal("__scene_transition_out__", this);
    scene.connect_signal("__scene_exit__", this);

    m_entry_overlap = true;
    m_entry_overlap_frame = true;

    m_current_room = Map::scene_to_room(scene);

    m_current_time = 0;

    glm::vec3 spawn_position;
    glm::vec3 dir = glm::vec3{0.0f, 0.0f, 1.0f};
    for (prt3::Door const & door : scene.get_all_components<prt3::Door>()) {
        if (door.id() == m_entry_door_id) {
            prt3::Node const & door_node = scene.get_node(door.node_id());
            glm::vec3 door_position =
                door_node.get_global_transform(scene).position;
            spawn_position =
                door_position +
                door.entry_offset() +
                m_player_door_offset;
            break;
        }
    }
    m_player_id = m_player_prefab.instantiate(scene, scene.get_root_id());
    prt3::Node & node = scene.get_node(m_player_id);
    node.set_global_position(scene, spawn_position);

    float yaw = std::atan2(dir.x, dir.z);

    glm::quat rot = glm::quat_cast(
        glm::eulerAngleYXZ(yaw, 0.0f, 0.0f)
    );

    node.set_global_rotation(scene, rot);

    prt3::PlayerController * controller =
        scene.get_script_from_node<prt3::PlayerController>(m_player_id);
    controller->deserialize_state(scene, m_player_state);

    m_camera_id = m_camera_prefab.instantiate(scene, scene.get_root_id());
    prt3::CameraController & cam =
        *scene.get_component<prt3::ScriptSet>(m_camera_id)
            .get_script<prt3::CameraController>(scene);

    cam.yaw() = m_cam_yaw;
    cam.pitch() = m_cam_pitch;

    m_canvas_id = scene.add_node_to_root("canvas");
    scene.add_component<prt3::Canvas>(m_canvas_id);

    // play soundtrack
    // if (scene.audio_manager().get_playing_midi() != m_midi) {
    //     scene.audio_manager().play_midi(m_midi, m_sound_font);
    // }
}

void GameState::on_init(prt3::Scene &) {
}

void GameState::on_update(prt3::Scene & scene, float) {
    m_npc_db.update(scene);
    m_current_time += dds::ms_per_frame;
}

void GameState::on_late_update(prt3::Scene & scene, float /*delta_time*/) {
    m_entry_overlap &= m_entry_overlap_frame;
    m_entry_overlap_frame = false;
    m_game_gui.on_update(scene, m_canvas_id, *this);
}

void GameState::register_door_overlap(prt3::Scene & scene, prt3::Door & door) {
    m_entry_overlap_frame = door.id() == m_entry_door_id;
    if (!m_entry_overlap) {
        scene.scene_manager()
            .queue_scene(door.destination_scene_path().data());
        m_exit_door_id = door.id();
        m_entry_door_id = door.destination_id();
    }
}

void GameState::init_resources(prt3::Scene & scene) {
    // sound
    m_midi =
        scene.audio_manager().load_midi("assets/audio/tracks/enter.mid");
    m_sound_font =
        scene.audio_manager()
             .load_sound_font("assets/audio/soundfonts/CT2MGM.sf2");
}
