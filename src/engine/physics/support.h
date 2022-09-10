#ifndef PRT3_SUPPORT_H
#define PRT3_SUPPORT_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace prt3::collision_util {

struct Triangle {
    union {
        struct {
            glm::vec3 a;
            glm::vec3 b;
            glm::vec3 c;
        };
        glm::vec3 data[3];
    };

    glm::vec3 & operator[](unsigned i) { return data[i]; }
};

struct Sphere {
    glm::vec3 position;
    float radius;
};

glm::vec3 calculate_furthest_point(Triangle triangle,
                                   glm::vec3 direction);

glm::vec3 calculate_furthest_point(Sphere sphere,
                                   glm::vec3 direction);

} // namespace prt3::collision_util

#endif
