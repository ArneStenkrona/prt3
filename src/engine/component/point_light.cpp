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
) : m_node_id{node_id} {
    in >> m_light;
}

namespace prt3 {

std::ostream & operator << (
    std::ostream & out,
    PointLightComponent const & component
) {
    out << component.light();

    return out;
}

} // namespace prt3
