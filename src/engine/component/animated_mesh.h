#ifndef PRT3_ANIMATED_MESH_COMPONENT_H
#define PRT3_ANIMATED_MESH_COMPONENT_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/resources.h"
#include "src/util/uuid.h"

namespace prt3 {

class Scene;

template<typename T>
class ComponentStorage;

class EditorContext;

class AnimatedMesh {
public:
    AnimatedMesh(Scene & scene, NodeID node_id);
    AnimatedMesh(
        Scene & scene,
        NodeID node_id,
        ResourceID resource_id,
        NodeID armature_id
    );
    AnimatedMesh(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ResourceID resource_id() const { return m_resource_id; }
    void set_resource_id(ResourceID id) { m_resource_id = id; }
    NodeID armature_id() const { return m_armature_id; }
    void set_armature_id(NodeID id) { m_armature_id = id; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Animated Mesh"; }
    static constexpr UUID uuid = 3983348536196717831ull;

private:
    NodeID m_node_id;
    ResourceID m_resource_id;
    NodeID m_armature_id = NO_NODE;

    void remove(Scene & /*scene*/) {}

    friend class ComponentStorage<AnimatedMesh>;
};

} // namespace prt3

#endif
