#ifndef PRT3_SHAPES_H
#define PRT3_SHAPES_H

#include "src/engine/physics/aabb.h"

#include "src/engine/component/transform.h"
#include "src/util/serialization_util.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>

namespace prt3 {

struct SweptTriangle {
    glm::vec3 a0;
    glm::vec3 b0;
    glm::vec3 c0;
    glm::vec3 a1;
    glm::vec3 b1;
    glm::vec3 c1;

    AABB aabb() const {
        AABB aabb;
        aabb.lower_bound = a0;
        aabb.upper_bound = a0;

        aabb.lower_bound = glm::min(aabb.lower_bound, b0);
        aabb.upper_bound = glm::max(aabb.upper_bound, b0);

        aabb.lower_bound = glm::min(aabb.lower_bound, c0);
        aabb.upper_bound = glm::max(aabb.upper_bound, c0);

        aabb.lower_bound = glm::min(aabb.lower_bound, a1);
        aabb.upper_bound = glm::max(aabb.upper_bound, a1);

        aabb.lower_bound = glm::min(aabb.lower_bound, b1);
        aabb.upper_bound = glm::max(aabb.upper_bound, b1);

        aabb.lower_bound = glm::min(aabb.lower_bound, c1);
        aabb.upper_bound = glm::max(aabb.upper_bound, c1);

        return aabb;
    }
};

struct Triangle {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;

    SweptTriangle sweep(glm::vec3 const & translation) const {
        return { a, b, c, a + translation, b + translation, c + translation };
    }
};

struct SweptSphere {
    glm::vec3 start;
    glm::vec3 end;
    float radius;

    AABB aabb() const {
        glm::vec3 min = glm::min(start, end);
        glm::vec3 max = glm::max(start, end);
        return { min - glm::vec3{radius},
                 max + glm::vec3{radius} };
    }
};

struct Sphere {
    glm::vec3 position;
    float radius;

    SweptSphere sweep(glm::vec3 const & translation) const {
        return SweptSphere{ position, position + translation, radius };
    }

    AABB aabb() const {
        return AABB{ position - glm::vec3{radius},
                     position + glm::vec3{radius} };
    }
};

inline std::ostream & operator << (
    std::ostream & out,
    Sphere const & sphere
) {
    write_stream(out, sphere.position);
    write_stream(out, sphere.radius);
    return out;
}

inline std::istream & operator >> (
    std::istream & in,
    Sphere & sphere
) {
    read_stream(in, sphere.position);
    read_stream(in, sphere.radius);
    return in;
}

template<size_t N>
struct DiscreteConvexHull {
    static_assert(N > 0);

    enum {
        Size = N
    };

    // No guarantee that all vertices are on the boundary
    std::array<glm::vec3, N> vertices;

    DiscreteConvexHull<2*N> sweep(glm::vec3 const & translation) const {
        DiscreteConvexHull<2*N> swept;
        for (size_t i = 0; i < vertices.size(); ++i) {
            swept.vertices[i] = vertices[i];
            swept.vertices[vertices.size() + i] = vertices[i] + translation;
        }
        return swept;
    }

    AABB aabb() const {
        AABB aabb;
        aabb.lower_bound = vertices[0];
        aabb.upper_bound = vertices[0];

        for (glm::vec3 const & vertex : vertices) {
            aabb.lower_bound = glm::min(aabb.lower_bound, vertex);
            aabb.upper_bound = glm::max(aabb.upper_bound, vertex);
        }

        return aabb;
    }
};


glm::vec3 calculate_furthest_point(Triangle const & triangle,
                                   glm::vec3 const & direction);

glm::vec3 calculate_furthest_point(SweptTriangle const & swept_triangle,
                                   glm::vec3 const & direction);

glm::vec3 calculate_furthest_point(Sphere const & sphere,
                                   glm::vec3 const & direction);

glm::vec3 calculate_furthest_point(SweptSphere const & swept_sphere,
                                   glm::vec3 const & direction);

template<size_t N>
glm::vec3 calculate_furthest_point(
    DiscreteConvexHull<N> const & hull,
    glm::vec3 const & direction
) {
    glm::vec3 max_point = hull.vertices[0];
    float max_dist = glm::dot(hull.vertices[0], direction);

    for (size_t i = 1; i < N; ++i) {
        float dist = glm::dot(hull.vertices[i], direction);
        if (dist > max_dist) {
            max_point = hull.vertices[i];
            max_dist = dist;
        }
    }

    return max_point;
}

template<typename ShapeA, typename ShapeB>
glm::vec3 calculate_support(ShapeA const & a,
                            ShapeB const & b,
                            glm::vec3 const & direction) {
    return calculate_furthest_point(a, direction)
         - calculate_furthest_point(b, -direction);
}

} // namespace prt3

#endif
