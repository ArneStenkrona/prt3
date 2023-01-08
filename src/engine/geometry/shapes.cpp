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

glm::vec3 calculate_furthest_point(SweptTriangle const & swept_triangle,
                                   glm::vec3 const & direction) {
    float dist_a0 = glm::dot(swept_triangle.a0, direction);
    float dist_b0 = glm::dot(swept_triangle.b0, direction);
    float dist_c0 = glm::dot(swept_triangle.c0, direction);
    float dist_a1 = glm::dot(swept_triangle.a1, direction);
    float dist_b1 = glm::dot(swept_triangle.b1, direction);
    float dist_c1 = glm::dot(swept_triangle.c1, direction);

    float max_dist = dist_a0;
    max_dist = glm::max(max_dist, dist_b0);
    max_dist = glm::max(max_dist, dist_c0);
    max_dist = glm::max(max_dist, dist_a1);
    max_dist = glm::max(max_dist, dist_b1);
    max_dist = glm::max(max_dist, dist_c1);

    if (max_dist == dist_a0) return swept_triangle.a0;
    if (max_dist == dist_b0) return swept_triangle.b0;
    if (max_dist == dist_c0) return swept_triangle.c0;
    if (max_dist == dist_a1) return swept_triangle.a1;
    if (max_dist == dist_b1) return swept_triangle.b1;

    return swept_triangle.c1;
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
