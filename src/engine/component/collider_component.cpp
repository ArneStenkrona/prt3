#include "collider_component.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    ColliderTag const & tag
)
 : m_node_id{node_id},
   m_tag{tag} {}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    Model const & model
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_mesh_collider(m_node_id, model);
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    Sphere const & sphere
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_sphere_collider(m_node_id, sphere);
}
