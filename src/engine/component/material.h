#ifndef PRT3_MATERIAL_COMPONENT_H
#define PRT3_MATERIAL_COMPONENT_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/render_data.h"
#include "src/util/uuid.h"

namespace prt3 {

class Scene;
class Mesh;

template<typename T>
class ComponentStorage;

class EditorContext;

class MaterialComponent {
public:
    MaterialComponent(Scene & scene, NodeID node_id);
    MaterialComponent(Scene & scene, NodeID node_id, ResourceID resource_id);
    MaterialComponent(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ResourceID resource_id() const { return m_resource_id; }
    void set_resource_id(ResourceID id) { m_resource_id = id; }

    MaterialOverride & material_override() { return m_material_override; }
    MaterialOverride const & material_override() const
    { return m_material_override; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Material"; }
    static constexpr UUID uuid = 11147784129203109440ull;

private:
    NodeID m_node_id;
    ResourceID m_resource_id;
    // material override is not serialized
    MaterialOverride m_material_override = {};

    void remove(Scene & /*scene*/) {}

    friend class ComponentStorage<MaterialComponent>;
};

} // namespace prt3

#endif
