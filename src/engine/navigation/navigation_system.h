#ifndef PRT3_NAVIGATION_SYSTEM_H
#define PRT3_NAVIGATION_SYSTEM_H

#include "src/engine/physics/aabb.h"
#include "src/engine/physics/aabb_tree.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <cstdint>
#include <vector>
#include <array>

namespace prt3 {

class Scene;

typedef int32_t NavMeshID;
constexpr NavMeshID NO_NAV_MESH = -1;

struct SubVec {
    uint32_t start_index;
    uint32_t num_indices;
};

class NavigationSystem {
public:
    NavMeshID generate_nav_mesh(
        Scene const & scene,
        CollisionLayer layer,
        float granularity,
        float max_edge_deviation,
        float max_edge_length,
        float min_height,
        float min_width
    );
private:
    struct NavigationMesh {
        std::vector<glm::vec3> vertices;
        std::vector<SubVec> adjacencies;
        std::vector<uint32_t> adjacency_indices;
        DynamicAABBTree aabb_tree;
    };

    std::vector<NavigationMesh> m_navigation_meshes;
};

} // namespace prt3

#endif // PRT3_NAVIGATION_SYSTEM_H
