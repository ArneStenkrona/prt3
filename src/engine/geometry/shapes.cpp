#include "shapes.h"

namespace prt3 {

glm::vec3 calculate_furthest_point(Triangle const & triangle,
                                   glm::vec3 const & direction) {
    float dist_a = glm::dot(triangle.a, direction);
    float dist_b = glm::dot(triangle.b, direction);
    float dist_c = glm::dot(triangle.c, direction);
    if (dist_a > dist_b && dist_a > dist_c) {
        return triangle.a;
    }
    if (dist_b > dist_c) {
        return triangle.b;
    }
    return triangle.c;
}

glm::vec3 calculate_furthest_point(Sphere const & sphere,
                                   glm::vec3 const & direction) {
    glm::vec3 dir = direction != glm::zero<glm::vec3>() ?
        glm::normalize(direction) : glm::vec3(1.0f, 0.0f, 0.0f);
    return sphere.position + sphere.radius * dir;
}

glm::vec3 calculate_furthest_point(SweptSphere const & swept_sphere,
                                   glm::vec3 const & direction) {
    glm::vec3 dir = direction != glm::zero<glm::vec3>() ?
        glm::normalize(direction) : glm::vec3(1.0f, 0.0f, 0.0f);

    float dist_start = glm::dot(swept_sphere.start, direction);
    float dist_end = glm::dot(swept_sphere.end, direction);
    if (dist_start > dist_end) {
        return swept_sphere.start + swept_sphere.radius * dir;
    }
    return swept_sphere.end + swept_sphere.radius * dir;
}

}
