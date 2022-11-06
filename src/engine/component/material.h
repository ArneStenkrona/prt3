#ifndef PRT3_COMPONENT_H
#define PRT3_COMPONENT_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/resources.h"

namespace prt3 {

class Scene;

class Material {
public:
    Material(Scene & scene, NodeID node_id, ResourceID resource_id);
    Material(Scene & scene, NodeID node_id, std::istream & in);

    Scene const & scene() const { return m_scene; }
    Scene & scene() { return m_scene; }
    NodeID node_id() const { return m_node_id; }
    ResourceID resource_id() const { return m_resource_id; }
private:
    Scene & m_scene;
    NodeID m_node_id;
    ResourceID m_resource_id;
};

std::ostream & operator << (
    std::ostream & out,
    Material const & component
);

} // namespace prt3

#endif
