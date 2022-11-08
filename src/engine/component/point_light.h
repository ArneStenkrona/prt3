#ifndef PRT3_POINT_LIGHT_H
#define PRT3_POINT_LIGHT_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/light.h"

namespace prt3 {

class Scene;

class PointLightComponent {
public:
    PointLightComponent(
        Scene & scene,
        NodeID node_id,
        PointLight const & light
    );
    PointLightComponent(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }

    PointLight const & light() const { return m_light; }
    PointLight & light() { return m_light; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

private:
    NodeID m_node_id;
    PointLight m_light;

    void remove(Scene & /*scene*/) {}

    friend class ComponentManager;
};

} // namespace prt3

#endif
