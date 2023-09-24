#include "door.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"
#include "src/daedalus/game_state/game_state.h"

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
    dds::GameState * game_state =
        scene.get_autoload_script<dds::GameState>();
    for (Door & door : components) {
        if (!scene.get_overlaps(door.node_id()).empty()) {
            game_state->register_door_overlap(scene, door);
            return;
        }
    }
}
