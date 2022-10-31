#ifndef PRT3_COLLIDER_COMPONENT_H
#define PRT3_COLLIDER_COMPONENT_H

#include "src/engine/physics/collider.h"
#include "src/engine/scene/node.h"
#include "src/engine/rendering/model.h"

namespace prt3 {

class Scene;

class ColliderComponent {
public:
    ColliderComponent(Scene & scene, NodeID node_id, ColliderTag const & tag);
    ColliderComponent(Scene & scene, NodeID node_id, Model const & model);
    ColliderComponent(Scene & scene, NodeID node_id, Sphere const & sphere);

    NodeID node_id() const { return m_node_id; }
    ColliderTag tag() const { return m_tag; }
private:
    NodeID m_node_id;
    ColliderTag m_tag;
};

} // namespace prt3

#endif
