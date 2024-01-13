#include "navigation_mesh.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"

using namespace prt3;

NavigationMeshComponent::NavigationMeshComponent(Scene &, NodeID node_id)
 : m_node_id{node_id},
   m_nav_mesh_id{NO_RESOURCE} {}

NavigationMeshComponent::NavigationMeshComponent(
    Scene &,
    NodeID node_id,
    NavMeshID nav_mesh_id
)
 : m_node_id{node_id},
   m_nav_mesh_id{nav_mesh_id} {}

NavigationMeshComponent::NavigationMeshComponent(
    Scene & scene,
    NodeID node_id,
    std::istream & in
)
 : m_node_id{node_id},
   m_nav_mesh_id{NO_NAV_MESH} {
    deserialize(in, scene);
}

void NavigationMeshComponent::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    NavigationSystem const & sys = scene.navigation_system();

    bool has_nav_mesh = m_nav_mesh_id != NO_NAV_MESH;
    write_stream(out, has_nav_mesh);
    if (has_nav_mesh) {
        sys.serialize_nav_mesh(m_nav_mesh_id, out);
    }
}

void NavigationMeshComponent::deserialize(
    std::istream & in,
    Scene & scene
) {
    NavigationSystem & sys = scene.navigation_system();
    if (m_nav_mesh_id != NO_NAV_MESH) {
        sys.remove_nav_mesh(m_nav_mesh_id);
        m_nav_mesh_id = NO_NAV_MESH;
    }

    bool has_nav_mesh;
    read_stream(in, has_nav_mesh);
    if (has_nav_mesh) {
        m_nav_mesh_id = sys.deserialize_nav_mesh(in);
    }
}
