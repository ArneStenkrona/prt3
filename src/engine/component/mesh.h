#ifndef PRT3_MESH_COMPONENT_H
#define PRT3_MESH_COMPONENT_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/resources.h"

namespace prt3 {

class Scene;
template<typename T>
class ComponentStorage;

class Mesh {
public:
    Mesh(Scene & scene, NodeID node_id);
    Mesh(Scene & scene, NodeID node_id, ResourceID resource_id);
    Mesh(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ResourceID resource_id() const { return m_resource_id; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Mesh"; }

private:
    NodeID m_node_id;
    ResourceID m_resource_id;

    void remove(Scene & /*scene*/) { /* TODO: perhaps ref-counting ? */}

    friend class ComponentStorage<Mesh>;
};

} // namespace prt3

#endif
