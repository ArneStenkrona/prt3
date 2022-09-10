#include "support.h"

glm::vec3 prt3::collision_util::calculate_furthest_point(
                                   Triangle triangle,
                                   glm::vec3 direction) {
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

glm::vec3 prt3::collision_util::calculate_furthest_point(
                                   Sphere sphere,
                                   glm::vec3 direction) {
    glm::vec3 dir = direction != glm::zero<glm::vec3>() ?
        glm::normalize(direction) : glm::vec3(1.0f, 0.0f, 0.0f);
    return sphere.position + sphere.radius * dir;
}
