#ifndef PRT3_GEOMETRY_UTIL_H
#define PRT3_GEOMETRY_UTIL_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace prt3 {

bool triangle_box_overlap(
    glm::vec3 const & box_center,
    glm::vec3 const & box_halfsize,
    glm::vec3 const & a,
    glm::vec3 const & b,
    glm::vec3 const & c
);

}

#endif // PRT3_GEOMETRY_UTIL_H
