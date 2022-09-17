#ifndef PRT3_COMPONENT_MANAGER_H
#define PRT3_COMPONENT_MANAGER_H

#include "src/engine/scene/node.h"

#include "src/engine/physics/physics_system.h"
#include "src/engine/rendering/light.h"
#include "src/engine/rendering/resources.h"

#include <unordered_map>

namespace prt3 {

class ComponentManager {
public:
    void set_mesh_component(NodeID node, ResourceID mesh)
        { m_mesh_components[node] = mesh; }
    void set_material_component(NodeID node, ResourceID material)
        { m_material_components[node] = material; }
    void set_point_light_component(NodeID node, PointLight const & light)
        { m_point_light_components[node] = light; }

    std::unordered_map<NodeID, ResourceID> const & get_mesh_components() const
        { return m_mesh_components; }
    std::unordered_map<NodeID, ResourceID> const & get_material_components() const
        { return m_material_components; }
    std::unordered_map<NodeID, PointLight> const & get_point_light_components() const
        { return m_point_light_components; }

private:
    std::unordered_map<NodeID, ResourceID> m_mesh_components;
    std::unordered_map<NodeID, ResourceID> m_material_components;
    std::unordered_map<NodeID, PointLight> m_point_light_components;
};

} // namespace prt3

#endif
