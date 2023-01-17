#ifndef PRT3_COLLIDER_CONTAINER
#define PRT3_COLLIDER_CONTAINER

#include "src/engine/physics/aabb_tree.h"
#include "src/engine/physics/collider.h"
#include "src/engine/geometry/shapes.h"
#include "src/engine/scene/node.h"

#include <unordered_map>

namespace prt3 {

template<typename Collider>
struct ColliderMap {
    std::unordered_map<ColliderID, Collider> map;
    ColliderID next_id = 0;
};

struct ColliderContainer {
    ColliderMap<MeshCollider> meshes;
    ColliderMap<SphereCollider> spheres;
    ColliderMap<BoxCollider> boxes;

    DynamicAABBTree aabb_tree;
};

} // namespace prt3

#endif // PRT3_COLLIDER_CONTAINER
