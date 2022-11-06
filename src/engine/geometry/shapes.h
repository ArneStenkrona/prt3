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

struct Triangle {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 c;
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


glm::vec3 calculate_furthest_point(Triangle const & triangle,
                                   glm::vec3 const & direction);

glm::vec3 calculate_furthest_point(Sphere const & sphere,
                                   glm::vec3 const & direction);

glm::vec3 calculate_furthest_point(SweptSphere const & swept_sphere,
                                   glm::vec3 const & direction);

template<typename ShapeA, typename ShapeB>
glm::vec3 calculate_support(ShapeA const & a,
                            ShapeB const & b,
                            glm::vec3 const & direction) {
    return calculate_furthest_point(a, direction)
         - calculate_furthest_point(b, -direction);
}

} // namespace prt3

#endif
