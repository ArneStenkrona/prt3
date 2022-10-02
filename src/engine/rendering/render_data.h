#ifndef PRT3_RENDER_DATA_H
#define PRT3_RENDER_DATA_H

#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/light.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <array>
#include <set>

namespace prt3 {

struct SceneRenderData {
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
    glm::vec3 view_position;
    glm::vec3 view_direction;
    float near_plane;
    float far_plane;
};

struct MeshRenderData {
    ResourceID mesh_id;
    ResourceID material_id;
    glm::mat4 transform;
};

struct PointLightRenderData {
    PointLight light;
    glm::vec3 position;
};

struct LightRenderData {
    static constexpr size_t MAX_NUMBER_OF_POINT_LIGHTS = 4;
    size_t number_of_point_lights;
    std::array<PointLightRenderData, MAX_NUMBER_OF_POINT_LIGHTS> point_lights;
    DirectionalLight directional_light;
    bool directional_light_on;
    AmbientLight ambient_light;
};

struct RenderData {
    SceneRenderData scene_data;
    std::vector<MeshRenderData> mesh_data;
    LightRenderData light_data;

    void clear() {
        mesh_data.resize(0);
        scene_data = {};
        light_data = {};
    }
};

}
#endif
