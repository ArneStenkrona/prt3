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
    glm::vec4 tint;
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

struct DecalRenderData {
    glm::mat4 transform;
    glm::vec4 color;
    ResourceID texture;
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

struct RenderRect2D {
    glm::vec4 color;
    glm::vec2 uv0;
    glm::vec2 uv1;
    glm::vec2 uv2;
    glm::vec2 uv3;
    glm::vec2 position;
    glm::vec2 dimension;
    ResourceID texture;
    int32_t layer;
};

struct ParticleAttributes {
    glm::vec4 pos_size; // (x, y, z, size)
    glm::vec2 base_uv;
    std::array<uint8_t, 4> color;
    float emissive;
};

struct ParticleData {
    std::vector<ParticleAttributes> attributes;

    struct TextureRange {
        uint32_t start_index;
        uint32_t count;
        glm::vec2 inv_div;
        ResourceID texture;
    };

    std::vector<TextureRange> textures;
};

struct SceneRenderData {
    std::vector<MeshRenderData> mesh_data;
    std::vector<AnimatedMeshRenderData> animated_mesh_data;
    std::vector<BoneData> bone_data;
    std::vector<MeshRenderData> selected_mesh_data;
    std::vector<AnimatedMeshRenderData> selected_animated_mesh_data;
    std::vector<DecalRenderData> decal_data;
    std::vector<RenderRect2D> canvas_data;
    ParticleData particle_data;
    LightRenderData light_data;
};

struct EditorRenderData {
    std::vector<WireframeRenderData> line_data;
};

struct RenderData {
    CameraRenderData camera_data;
    SceneRenderData scene;
    EditorRenderData editor_data;

    void clear() {
        camera_data = {};
        scene.light_data = {};
        scene.mesh_data.clear();
        scene.animated_mesh_data.clear();
        scene.bone_data.clear();
        scene.selected_mesh_data.clear();
        scene.selected_animated_mesh_data.clear();
        scene.decal_data.clear();
        scene.canvas_data.clear();
        scene.particle_data.attributes.clear();
        scene.particle_data.textures.clear();
        editor_data.line_data.clear();
    }
};

}
#endif
