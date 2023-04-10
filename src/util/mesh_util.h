#ifndef PRT3_MESH_UTIL_H
#define PRT3_MESH_UTIL_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>

namespace prt3 {

void push_back_box(
    glm::vec3 const & lower_bound,
    glm::vec3 const & upper_bound,
    std::vector<glm::vec3> & buffer
);

} // namespace prt3

#endif // PRT3_MESH_UTIL_H