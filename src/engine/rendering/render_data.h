#ifndef PRT3_RENDER_DATA_H
#define PRT3_RENDER_DATA_H

#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/light.h"
#include "src/engine/scene/node.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <array>


namespace prt3 {

struct CameraRenderData {
    glm::mat4 view_matrix;
    glm::mat4 projection_matrix;
    glm::vec3 view_position;
    glm::vec3 view_direction;
    float near_plane;
    float far_plane;
};

struct NodeData {
    NodeID id;
    bool selected;
};

struct MaterialOverride {
    bool tint_active;
    glm::vec3 tint;
};

struct MeshRenderData {
    ResourceID mesh_id;
    ResourceID material_id;
    MaterialOverride material_override;
    NodeData node_data;
    glm::mat4 transform;
};

struct AnimatedMeshRenderData {
    MeshRenderData mesh_data;
    uint32_t bone_data_index;
};

struct BoneData {
    std::array<glm::mat4, 100> bones;
};

struct WireframeRenderData {
    ResourceID mesh_id;
    glm::vec4 color;
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

struct WorldRenderData {
    std::vector<MeshRenderData> mesh_data;
    std::vector<AnimatedMeshRenderData> animated_mesh_data;
    std::vector<BoneData> bone_data;
    std::vector<MeshRenderData> selected_mesh_data;
    std::vector<AnimatedMeshRenderData> selected_animated_mesh_data;
    LightRenderData light_data;
};

struct EditorRenderData {
    std::vector<WireframeRenderData> line_data;
};

struct RenderData {
    CameraRenderData camera_data;
    WorldRenderData world;
    EditorRenderData editor_data;

    void clear() {
        camera_data = {};
        world.light_data = {};
        world.mesh_data.resize(0);
        world.animated_mesh_data.resize(0);
        world.bone_data.resize(0);
        world.selected_mesh_data.resize(0);
        world.selected_animated_mesh_data.resize(0);
        editor_data.line_data.resize(0);
    }
};

}
#endif
