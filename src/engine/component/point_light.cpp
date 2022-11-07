#include "point_light.h"

#include "src/util/serialization_util.h"

using namespace prt3;

PointLightComponent::PointLightComponent(
    Scene &,
    NodeID node_id,
    PointLight const & light
)
 : m_node_id{node_id},
   m_light{light} {}

PointLightComponent::PointLightComponent(
    Scene &,
    NodeID node_id,
    std::istream & in
)
 : m_node_id{node_id} {
    in >> m_light;
}

void PointLightComponent::serialize(
    std::ostream & out,
    Scene const &
) const {
    out << m_light;
}
