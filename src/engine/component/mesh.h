#ifndef PRT3_MESH_H
#define PRT3_MESH_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/resources.h"

namespace prt3 {

class Scene;

class Mesh {
public:
    Mesh(Scene & scene, NodeID node_id, ResourceID resource_id);
    Mesh(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ResourceID resource_id() const { return m_resource_id; }

    Scene const & scene() const { return m_scene; }
    Scene & scene() { return m_scene; }

private:
    Scene & m_scene;
    NodeID m_node_id;
    ResourceID m_resource_id;
};

std::ostream & operator << (
    std::ostream & out,
    Mesh const & component
);

} // namespace prt3

#endif
