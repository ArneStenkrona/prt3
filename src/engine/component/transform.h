#ifndef PRT3_TRANSFORM_H
#define PRT3_TRANSFORM_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace prt3 {

struct Transform {
    glm::quat rotation = {1.0f,0.0f,0.0f,0.0f};
    glm::vec3 position = {0.0f,0.0f,0.0f};
    glm::vec3 scale = {1.0f,1.0f,1.0f};

    inline glm::vec3 get_front() const { return rotation * glm::vec3{ 0.0, 0.0, 1.0}; }
    inline glm::vec3 get_up() const { return rotation * glm::vec3{ 0.0, 1.0, 0.0}; }
    inline glm::vec3 get_right() const { return rotation * glm::vec3{ -1.0, 0.0, 0.0}; }

    glm::mat4 to_matrix() const {
        glm::mat4 scaleM = glm::scale(scale);
        glm::mat4 rotateM = glm::toMat4(glm::normalize(rotation));
        glm::mat4 translateM = glm::translate(glm::mat4(1.0f), position);
        return translateM * rotateM * scaleM;
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

} // namespace prt3

#endif
