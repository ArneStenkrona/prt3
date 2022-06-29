#ifndef PRT3_NODE_H
#define PRT3_NODE_H

#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/scene/transform.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>

namespace prt3 {

typedef int NodeID;
constexpr NodeID NO_NODE = -1;

struct Node {
    Transform local_transform;
    ResourceID mesh_id = prt3::NO_RESOURCE;
    ResourceID material_id = prt3::NO_RESOURCE;
    NodeID parent = NO_NODE;
    std::vector<NodeID> children;

    bool should_be_rendered() const { return mesh_id != prt3::NO_RESOURCE; }
};

}
#endif
