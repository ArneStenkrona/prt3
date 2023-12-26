#include "game_state.h"

#include "src/daedalus/input_mapping/input_mapping.h"

using namespace dds;

#define MAP_PATH "assets/models/map/island.map" // hard-coded for now

GameState::GameState(prt3::Scene & scene, prt3::NodeID node_id)
 : Script(scene, node_id),
   m_map{MAP_PATH},
   m_npc_db{*this},
   m_item_db{*this},
   m_object_db{*this},
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
   m_item_db{*this},
   m_object_db{*this},
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

        prt3::Camera & cam = scene.get_camera();
        prt3::Node & player = scene.get_node(m_player_id);

        uint32_t exit_door_id =
            m_map.local_to_global_door_id(m_current_room, m_exit_door_id);
        uint32_t entry_door_id = m_map.get_door_destination_id(exit_door_id);

        glm::vec3 player_pos = player.get_global_transform(scene).position;
        glm::vec3 entry_pos = m_map.get_door_entry_position(entry_door_id) +
                              m_player_door_offset;
        glm::vec3 diff = entry_pos - player_pos;

        glm::vec3 translation = t * dt * diff;
        cam.transform().position += translation;
        player.translate_node(scene, translation);

    } else if (signal == "__scene_exit__") {
        PlayerController * controller =
            scene.get_script_from_node<PlayerController>(m_player_id);
        m_player_state = controller->serialize_state(scene);

        prt3::CameraController & cam =
            *scene.get_component<prt3::ScriptSet>(m_camera_id)
                .get_script<prt3::CameraController>(scene);

        m_cam_yaw = cam.yaw();
        m_cam_pitch = cam.pitch();

        for (prt3::Door const & door : scene.get_all_components<prt3::Door>()) {
            if (door.id() == m_exit_door_id) {
                m_player_door_offset =
                    get_door_local_position(scene, door, m_player_id);
                break;
            }
        }

        m_npc_db.on_scene_exit();
    }
}

glm::vec3 GameState::get_door_local_position(
    prt3::Scene const & scene,
    prt3::Door const & door,
    prt3::NodeID node_id
) const {
    prt3::Node const & door_node = scene.get_node(door.node_id());
    prt3::Transform door_tform =
        door_node.get_global_transform(scene);
    glm::vec3 door_position = door_tform.position;
    glm::vec3 entry_position = door_position + door.entry_offset();

    prt3::Node const & node = scene.get_node(node_id);
    glm::vec3 pos = node.get_global_transform(scene).position;
    glm::vec3 v = pos - entry_position;

    glm::vec3 n = door_tform.get_up();
    return v - glm::dot(v, n) * n;
}

glm::vec3 GameState::get_door_local_position(
    uint32_t door_id,
    NPCID npc_id
) const {
    glm::vec3 pos = m_npc_db.get_npc(npc_id).map_position.position;
    glm::vec3 v = pos - m_map.get_door_entry_position(door_id);
    glm::vec3 n = m_map.get_door_up(door_id);
    return v - glm::dot(v, n) * n;
}

void GameState::on_start(prt3::Scene & scene) {
    scene.connect_signal("__scene_transition_out__", this);
    scene.connect_signal("__scene_exit__", this);

    m_interactable = nullptr;

    m_entry_overlap = true;
    m_entry_overlap_frame = true;

    m_current_room = Map::scene_to_room(scene);

    glm::vec3 spawn_position{};
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
    prt3::Prefab const & player_prefab = m_prefab_db.get(PrefabDB::player);
    m_player_id = player_prefab.instantiate(scene, scene.root_id());
    prt3::Node & player = scene.get_node(m_player_id);
    player.set_global_position(scene, spawn_position);

    PlayerController * controller =
        scene.get_script_from_node<PlayerController>(m_player_id);
    controller->deserialize_state(scene, m_player_state);

    prt3::Prefab const & camera_prefab = m_prefab_db.get(PrefabDB::camera);
    m_camera_id = camera_prefab.instantiate(scene, scene.root_id());
    prt3::CameraController & cam =
        *scene.get_component<prt3::ScriptSet>(m_camera_id)
            .get_script<prt3::CameraController>(scene);

    cam.yaw() = m_cam_yaw;
    cam.pitch() = m_cam_pitch;
    cam.set_target(m_player_id);

    m_canvas_id = scene.add_node_to_root("canvas");
    scene.add_component<prt3::Canvas>(m_canvas_id);

    // play soundtrack
    // if (scene.audio_manager().get_playing_midi() != m_midi) {
    //     scene.audio_manager().play_midi(m_midi, m_sound_font);
    // }

    m_object_db.on_scene_start();
    m_npc_db.on_scene_start(scene);

    prt3::AnimationID anim_id =
        scene.get_component<prt3::Armature>(m_player_id).animation_id();
    scene.animation_system().update_transforms(scene, anim_id);
}

void GameState::on_update(prt3::Scene & scene, float) {
    m_npc_db.update(scene);
    m_item_db.update();
    m_object_db.update(scene);
    m_current_time += dds::ms_per_frame;
}

void GameState::on_late_update(prt3::Scene & scene, float delta_time) {
    m_entry_overlap &= m_entry_overlap_frame;
    m_entry_overlap_frame = false;
    m_game_gui.on_update(scene, m_canvas_id, *this, delta_time);

    /* handle interacts */
    if (m_interactable != nullptr &&
        scene.get_input().get_key_down(dds::input_mapping::interact)) {
        m_interactable->interact(scene);
    }
    m_interactable = nullptr;
}

void GameState::on_game_end(prt3::Scene & scene) {
    m_game_gui.free_resources(scene);
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

void GameState::register_interactable(Interactable & candidate) {
    if (m_interactable == nullptr ||
        m_interactable->priority() < candidate.priority()) {
        m_interactable = &candidate;
    }
}

void GameState::init_resources(prt3::Scene & scene) {
    m_current_time = 0;

    // sound
    m_midi =
        scene.audio_manager().load_midi("assets/audio/tracks/enter.mid");
    m_sound_font =
        scene.audio_manager()
             .load_sound_font("assets/audio/soundfonts/CT2MGM.sf2");

    // player state
    m_player_state.clip_a.animation_index = prt3::NO_ANIMATION;
    m_player_state.clip_b.animation_index = prt3::NO_ANIMATION;
}
