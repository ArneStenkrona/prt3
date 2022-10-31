#ifndef PRT3_MESH_H
#define PRT3_MESH_H

#include "src/engine/rendering/model_manager.h"
#include "src/engine/rendering/resources.h"


namespace prt3 {

class Mesh {
public:
    Mesh(Scene & scene, NodeID node_id, ResourceID resource_id);

    ResourceID resource_id() const { return m_resource_id; }
    ResourceID node_id() const { return m_node_id; }

private:
    ResourceID m_resource_id;
    NodeID m_node_id;
};

} // namespace prt3

#endif
