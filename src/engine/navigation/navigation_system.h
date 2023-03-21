#ifndef PRT3_NAVIGATION_SYSTEM_H
#define PRT3_NAVIGATION_SYSTEM_H

#include <cstdint>

namespace prt3 {

typedef int32_t NavMeshID;
constexpr NavMeshID NO_NAV_MESH = -1;

class NavigationSystem {
public:

    NavMeshID generate_nav_mesh(
        float granularity,
        float min_height,
        glm::vec3 const * geometry_data,
        size_t n_geometry
    );
private:
};

} // namespace prt3

#endif // PRT3_NAVIGATION_SYSTEM_H
