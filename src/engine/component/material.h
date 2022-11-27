#ifndef PRT3_MATERIAL_COMPONENT_H
#define PRT3_MATERIAL_COMPONENT_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/resources.h"

namespace prt3 {

class Scene;
class Mesh;

template<typename T>
class ComponentStorage;

class EditorContext;
template<typename T>
void inner_show_component(EditorContext &, NodeID);

class MaterialComponent {
public:
    MaterialComponent(Scene & scene, NodeID node_id);
    MaterialComponent(Scene & scene, NodeID node_id, ResourceID resource_id);
    MaterialComponent(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ResourceID resource_id() const { return m_resource_id; }
    void set_resource_id(ResourceID id) { m_resource_id = id; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Material"; }

private:
    NodeID m_node_id;
    ResourceID m_resource_id;

    void remove(Scene & /*scene*/) {}

    friend class ComponentStorage<MaterialComponent>;

    friend void inner_show_component<Mesh>(EditorContext &, NodeID);
};

} // namespace prt3

#endif
