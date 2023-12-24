#ifndef DDS_GAME_STATE_H
#define DDS_GAME_STATE_H

#include "src/daedalus/game_state/game_time.h"
#include "src/daedalus/gui/game_gui.h"
#include "src/daedalus/map/map.h"
#include "src/daedalus/npc/npc_db.h"
#include "src/daedalus/objects/interactable.h"

#include "src/engine/component/script/script.h"
#include "src/daedalus/character/player_controller.h"
#include "src/daedalus/game_state/item_db.h"
#include "src/daedalus/game_state/object_db.h"
#include "src/daedalus/game_state/prefab_db.h"
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

namespace dds {

class GameState : public prt3::Script {
public:
    explicit GameState(prt3::Scene & scene, prt3::NodeID node_id);

    explicit GameState(
        std::istream &,
        prt3::Scene & scene,
        prt3::NodeID m_node_id
    );

    virtual void on_signal(
        prt3::Scene & scene,
        prt3::SignalString const & signal,
        void * data
    );

    virtual void on_start(prt3::Scene & scene);
    virtual void on_update(prt3::Scene &, float);
    virtual void on_late_update(prt3::Scene &, float);
    virtual void on_game_end(prt3::Scene & scene);

    void set_exit_door_id(prt3::DoorID id) { m_exit_door_id = id; }
    void set_entry_door_id(prt3::DoorID id) { m_entry_door_id = id; }

    void register_door_overlap(prt3::Scene & scene, prt3::Door & door);
    void register_interactable(Interactable & candidate);

    Interactable const * interactable() const { return m_interactable; }

    Map & map() { return m_map; }
    NPCDB & npc_db() { return m_npc_db; }
    ItemDB const & item_db() const { return m_item_db; }
    ItemDB & item_db() { return m_item_db; }
    PrefabDB const & prefab_db() const { return m_prefab_db; }
    ObjectDB const & object_db() const { return m_object_db; }
    ObjectDB & object_db() { return m_object_db; }

    TimeMS current_time() const { return m_current_time; }

    RoomID current_room() const { return m_current_room; }

    prt3::NodeID player_id() const { return m_player_id; }

private:
    Map m_map;
    NPCDB m_npc_db;
    ItemDB m_item_db;
    PrefabDB m_prefab_db;
    ObjectDB m_object_db;
    RoomID m_current_room = 0;
    GameGui m_game_gui;

    TimeMS m_current_time = 0;

    prt3::DoorID m_exit_door_id = 0;
    prt3::DoorID m_entry_door_id = 0;
    bool m_entry_overlap_frame;
    bool m_entry_overlap;

    Interactable * m_interactable = nullptr;

    prt3::NodeID m_player_id;
    CharacterController::SerializedState m_player_state = {};
    glm::vec3 m_player_door_offset;

    prt3::NodeID m_camera_id;

    prt3::NodeID m_canvas_id;

    float m_cam_yaw;
    float m_cam_pitch;

    prt3::MidiID m_midi;
    prt3::SoundFontID m_sound_font;

    void init_resources(prt3::Scene & scene);

REGISTER_SCRIPT(GameState, game_state, 11630114958491958378)
};

} // namespace dds

#endif // DDS_GAME_STATE_H
