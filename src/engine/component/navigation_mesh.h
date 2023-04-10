#ifndef PRT3_NAVIGATION_MESH_COMPONENT_H
#define PRT3_NAVIGATION_MESH_COMPONENT_H

#include "src/engine/scene/node.h"
#include "src/engine/navigation/navigation_system.h"
#include "src/util/uuid.h"

namespace prt3 {

class Scene;
class EditorContext;

template<typename T>
class ComponentStorage;

template<typename T>
void inner_show_component(EditorContext &, NodeID);

class NavigationMeshComponent {
public:
    NavigationMeshComponent(Scene & scene, NodeID node_id);
    NavigationMeshComponent(Scene & scene, NodeID node_id, NavMeshID nav_mesh_id);
    NavigationMeshComponent(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    NavMeshID nav_mesh_id() const { return m_nav_mesh_id; }
    void nav_mesh_id(NavMeshID id) { m_nav_mesh_id = id; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Navigation Mesh"; }
    static constexpr UUID uuid = 12863741067030180881ull;

private:
    NodeID m_node_id;
    ResourceID m_nav_mesh_id;

    void remove(Scene & /*scene*/) {}

    friend class ComponentStorage<NavigationMeshComponent>;
    friend void inner_show_component<NavigationMeshComponent>(EditorContext &, NodeID);
};

} // namespace prt3

#endif
