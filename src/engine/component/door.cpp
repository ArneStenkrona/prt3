#include "door.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"
#include "src/engine/component/script/game_state.h"

using namespace prt3;

Door::Door(Scene & /*scene*/, NodeID node_id)
 : m_node_id{node_id} {}

Door::Door(Scene & /*scene*/, NodeID node_id, std::istream & in)
 : m_node_id{node_id} {
    read_stream(in, m_id);
    read_stream(in, m_destination_id);
    read_stream(in, m_entry_offset);
    read_c_string(
        in,
        m_destination_scene_path.data(),
        m_destination_scene_path.writeable_size()
    );
}

void Door::serialize(
    std::ostream & out,
    Scene const & /*scene*/
) const {
    write_stream(out, m_id);
    write_stream(out, m_destination_id);
    write_stream(out, m_entry_offset);
    write_c_string(out, m_destination_scene_path.data());
}

void Door::update(Scene & scene, std::vector<Door> & components) {
    for (Door & door : components) {
        if (!scene.get_overlaps(door.node_id()).empty()) {
            scene.scene_manager()
                .queue_scene(door.m_destination_scene_path.data());
            GameState * game_state = scene.get_autoload_script<GameState>();
            game_state->set_entry_door_id(door.id());
            return;
        }
    }
}
