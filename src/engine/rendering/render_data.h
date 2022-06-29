#ifndef PRT3_RENDER_DATA_H
#define PRT3_RENDER_DATA_H

#include "src/engine/rendering/resources.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace prt3 {

struct RenderData {
    ResourceID mesh_rid;
    ResourceID material_rid;
    glm::mat4 transform;
};

}
#endif
