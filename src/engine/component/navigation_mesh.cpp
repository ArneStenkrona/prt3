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
    Scene & /*scene*/,
    NodeID node_id,
    std::istream & /*in*/
)
 : m_node_id{node_id},
   m_nav_mesh_id{NO_NAV_MESH} {
}

void NavigationMeshComponent::serialize(
    std::ostream & /*out*/,
    Scene const & /*scene*/
) const {
}
