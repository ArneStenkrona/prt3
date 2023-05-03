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

bool triangle_ray_intersect(
    glm::vec3 origin,
    glm::vec3 ray_vector,
    glm::vec3 a,
    glm::vec3 b,
    glm::vec3 c,
    glm::vec3 & intersection
);

void triangle_segment_clip_2d(
    glm::vec2 p0,
    glm::vec2 p1,
    glm::vec2 a,
    glm::vec2 b,
    glm::vec2 c,
    glm::vec2 & t0,
    glm::vec2 & t1
);

glm::vec3 closest_point_on_triangle(
    glm::vec3 const & p,
    glm::vec3 const & a,
    glm::vec3 const & b,
    glm::vec3 const & c
);

}

#endif // PRT3_GEOMETRY_UTIL_H
