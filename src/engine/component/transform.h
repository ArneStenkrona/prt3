#ifndef PRT3_TRANSFORM_H
#define PRT3_TRANSFORM_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "src/util/serialization_util.h"

namespace prt3 {

struct Transform {
    glm::quat rotation = {1.0f,0.0f,0.0f,0.0f};
    glm::vec3 position = {0.0f,0.0f,0.0f};
    glm::vec3 scale = {1.0f,1.0f,1.0f};

    inline glm::vec3 get_front() const { return rotation * glm::vec3{ 0.0, 0.0, 1.0}; }
    inline glm::vec3 get_up() const { return rotation * glm::vec3{ 0.0, 1.0, 0.0}; }
    inline glm::vec3 get_right() const { return rotation * glm::vec3{ -1.0, 0.0, 0.0}; }

    glm::mat4 to_matrix() const {
        glm::mat4 rotateM = glm::toMat4(glm::normalize(rotation));
        glm::mat4 scaleM = glm::scale(scale);
        glm::mat4 translateM = glm::translate(glm::mat4(1.0f), position);
        return translateM * scaleM * rotateM;
    }

    Transform & from_matrix(glm::mat4 const & matrix) {
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(matrix, scale, rotation, position, skew, perspective);
        rotation = glm::conjugate(rotation);
        return *this;
    }
};

inline bool operator==(Transform const & lhs, Transform const & rhs) {
    return lhs.rotation == rhs.rotation &&
           lhs.position == rhs.position &&
           lhs.scale    == rhs.scale;
}

inline bool operator!=(Transform const & lhs, Transform const & rhs) {
    return lhs.rotation != rhs.rotation ||
           lhs.position != rhs.position ||
           lhs.scale    != rhs.scale;
}

inline std::ostream & operator << (
    std::ostream & out,
    Transform const & transform
) {
    write_stream(out, transform.rotation);
    write_stream(out, transform.position);
    write_stream(out, transform.scale);
    return out;
}

inline std::istream & operator >> (std::istream & in, Transform & transform) {
    read_stream(in, transform.rotation);
    read_stream(in, transform.position);
    read_stream(in, transform.scale);
    return in;
}

} // namespace prt3

#endif
