#ifndef PRT3_COLLIDER_COMPONENT_H
#define PRT3_COLLIDER_COMPONENT_H

#include "src/engine/physics/collider.h"
#include "src/engine/scene/node.h"
#include "src/engine/rendering/model.h"
#include "src/util/serialization_util.h"

#include <iostream>

namespace prt3 {

class Scene;

class ColliderComponent {
public:
    ColliderComponent(Scene & scene, NodeID node_id, Model const & model);
    ColliderComponent(Scene & scene, NodeID node_id, Sphere const & sphere);
    ColliderComponent(Scene & scene, NodeID node_id, std::istream & in);

    Scene const & scene() const { return m_scene; }
    Scene & scene() { return m_scene; }
    NodeID node_id() const { return m_node_id; }
    ColliderTag tag() const { return m_tag; }
private:
    Scene & m_scene;
    NodeID m_node_id;
    ColliderTag m_tag;
};

std::ostream & operator << (
    std::ostream & out,
    ColliderComponent const & component
);

} // namespace prt3

#endif
