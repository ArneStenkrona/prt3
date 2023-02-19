#ifndef PRT3_DOOR_H
#define PRT3_DOOR_H

#include "src/engine/scene/node.h"
#include "src/util/uuid.h"
#include "src/util/fixed_string.h"

namespace prt3 {

class Scene;

template<typename T>
class ComponentStorage;

class EditorContext;

template<typename T>
void inner_show_component(EditorContext &, NodeID);

class ComponentManager;

typedef int32_t DoorID;
typedef FixedString<256> DoorPathType;
static constexpr DoorID NO_DOOR = -1;

class Door {
public:
    Door(Scene & scene, NodeID node_id);
    Door(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    DoorID const & id() const { return m_id; }
    DoorID & id() { return m_id; }

    DoorID const & destination_id() const { return m_destination_id; }
    DoorID & destination_id() { return m_destination_id; }

    FixedString<256> const & destination_scene_path() const
    { return m_destination_scene_path; }
    FixedString<256> & destination_scene_path()
    { return m_destination_scene_path; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Door"; }
    static constexpr UUID uuid = 5669492098565304587ull;

private:
    NodeID m_node_id;

    DoorID m_id = NO_DOOR;
    DoorID m_destination_id = NO_DOOR;
    DoorPathType m_destination_scene_path;

    void remove(Scene & /*scene*/) {}

    static void update(Scene & scene, std::vector<Door> & components);

    friend class ComponentStorage<Door>;
    friend class ComponentManager;
    friend void inner_show_component<Door>(EditorContext &, NodeID);
};

} // namespace prt3

#endif
