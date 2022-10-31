#include "point_light.h"

using namespace prt3;

PointLightComponent::PointLightComponent(
    Scene &,
    NodeID node_id,
    PointLight const & light
)
 : m_node_id{node_id},
   m_light{light} {}

