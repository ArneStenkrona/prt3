#ifndef PRT3_LIGHT_H
#define PRT3_LIGHT_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include "src/util/serialization_util.h"

namespace prt3 {

struct PointLight {
    alignas(16) glm::vec3 color;
    alignas(4) float quadratic_term;
    alignas(4) float linear_term;
    alignas(4) float constant_term;
};

inline std::ostream & operator << (
    std::ostream & out,
    PointLight const & light
) {
    write_stream(out, light.color);
    write_stream(out, light.quadratic_term);
    write_stream(out, light.linear_term);
    write_stream(out, light.constant_term);
    return out;
}

inline std::istream & operator >> (
    std::istream & in,
    PointLight & light
) {
    read_stream(in, light.color);
    read_stream(in, light.quadratic_term);
    read_stream(in, light.linear_term);
    read_stream(in, light.constant_term);
    return in;
}

struct DirectionalLight {
    alignas(16) glm::vec3 direction;
    alignas(16) glm::vec3 color;
};

inline std::ostream & operator << (
    std::ostream & out,
    DirectionalLight const & light
) {
    write_stream(out, light.direction);
    write_stream(out, light.color);
    return out;
}

inline std::istream & operator >> (
    std::istream & in,
    DirectionalLight & light
) {
    read_stream(in, light.direction);
    read_stream(in, light.color);
    return in;
}

struct AmbientLight {
    glm::vec3 color = glm::vec3{1.0f};
};

inline std::ostream & operator << (
    std::ostream & out,
    AmbientLight const & light
) {
    write_stream(out, light.color);
    return out;
}

inline std::istream & operator >> (
    std::istream & in,
    AmbientLight & light
) {
    read_stream(in, light.color);
    return in;
}

} // namespace prt3

#endif
