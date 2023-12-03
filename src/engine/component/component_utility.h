#ifndef PRT3_COMPONENT_UTILITY_H
#define PRT3_COMPONENT_UTILITY_H

#include "src/engine/scene/scene.h"

namespace prt3 {

void serialize_texture(
    std::ostream & out,
    Scene const & scene,
    ResourceID tex_id
);

ResourceID deserialize_texture(std::istream & in, Scene & scene);

} // namespace prt3

#endif // PRT3_COMPONENT_UTILITY_H
