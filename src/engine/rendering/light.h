#ifndef PRT3_LIGHT_H
#define PRT3_LIGHT_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

namespace prt3 {

struct PointLight {
    alignas(16) glm::vec3 color;
    alignas(4) float quadratic_term;
    alignas(4) float linear_term;
    alignas(4) float constant_term;
};

struct DirectionalLight {
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 color;
};

struct AmbientLight {
    glm::vec3 color;
};

} // namespace prt3

#endif
