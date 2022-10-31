#ifndef PRT3_COMPONENT_H
#define PRT3_COMPONENT_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/resources.h"

namespace prt3 {

class Scene;

class Material {
public:
    Material(Scene & scene, NodeID node_id, ResourceID resource_id);

    ResourceID node_id() const { return m_node_id; }
    ResourceID resource_id() const { return m_resource_id; }
private:
    NodeID m_node_id;
    ResourceID m_resource_id;
};

} // namespace prt3

#endif
