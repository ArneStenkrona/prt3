#ifndef PRT3_RENDER_DATA_H
#define PRT3_RENDER_DATA_H

#include "src/engine/rendering/resources.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>

namespace prt3 {

struct SceneRenderData {
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
};

struct MeshRenderData {
    ResourceID mesh_id;
    ResourceID material_id;
    glm::mat4 transform;
};

struct RenderData {
    SceneRenderData scene_data;
    std::vector<MeshRenderData> mesh_data;
};

}
#endif
