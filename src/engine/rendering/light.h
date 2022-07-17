#ifndef PRT3_LIGHT_H
#define PRT3_LIGHT_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace prt3 {

struct PointLight {
    glm::vec3 color;
    float quadratic_term;
    float linear_term;
    float constant_term;
};

} // namespace prt3

#endif
