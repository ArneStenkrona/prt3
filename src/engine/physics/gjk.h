#ifndef PRT3_COLLISION_H
#define PRT3_COLLISION_H

#include "src/engine/physics/support.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>
#include <cassert>

// Thanks https://blog.winter.dev/2020/gjk-algorithm/ for
// wonderful implementation reference of GJK

namespace prt3::collision_util {

bool next_simplex_line(std::array<glm::vec3, 4> & points,
                       unsigned int & n_points,
                       glm::vec3 & direction);

bool next_simplex_triangle(std::array<glm::vec3, 4> & points,
                           unsigned int & n_points,
                           glm::vec3 & direction);

bool next_simplex_tetrahedron(std::array<glm::vec3, 4> & points,
                              unsigned int & n_points,
                              glm::vec3 & direction);

template<typename ShapeA, typename ShapeB>
glm::vec3 calculate_support(ShapeA const & a,
                            ShapeB const & b,
                            glm::vec3 direction) {
    return calculate_furthest_point(a, direction)
         - calculate_furthest_point(b, -direction);
}

template<typename ShapeA, typename ShapeB>
bool gjk(ShapeA const & a,
         ShapeB const & b) {
    glm::vec3 support = calculate_support(a, b, glm::vec3{1.0f, 0.0f, 0.0f});

    unsigned int n_points = 1;
    std::array<glm::vec3, 4> points;
    points[0] = support;

    glm::vec3 direction = -support;

    while (true) {
        support = calculate_support(a, b, direction);
        if (glm::dot(support, direction) <= 0.0f) {
            return false;
        }

        points[3] = points[2];
        points[2] = points[1];
        points[1] = points[0];
        points[0] = support;
        n_points = n_points == 4 ? 4 : n_points + 1;

        bool res_simplex = false;

        switch (n_points) {
            case 2: {
                res_simplex = next_simplex_line(points,
                                                n_points,
                                                direction);
                break;
            }
            case 3: {
                res_simplex = next_simplex_triangle(points,
                                                    n_points,
                                                    direction);
                break;
            }
            case 4: {
                res_simplex = next_simplex_tetrahedron(points,
                                                       n_points,
                                                       direction);
                break;
            }
            default: {
                assert(false && "Invalid simplex");
            }
        }

        if (res_simplex) {
            return true;
        }
    }
}

} // namespace prt3::collision_util

#endif
