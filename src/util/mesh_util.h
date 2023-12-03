#ifndef PRT3_MESH_UTIL_H
#define PRT3_MESH_UTIL_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace prt3 {

// geo needs space for 36 vec3's
void insert_box(
    glm::vec3 const & lower_bound,
    glm::vec3 const & upper_bound,
    glm::vec3 * geo
);

// geo needs space for 24 vec3's
void insert_line_box(
    glm::vec3 const & lower_bound,
    glm::vec3 const & upper_bound,
    glm::vec3 * geo
);

} // namespace prt3

#endif // PRT3_MESH_UTIL_H