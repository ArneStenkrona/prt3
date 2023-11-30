#ifndef PRT3_MODEL_H
#define PRT3_MODEL_H

#include "src/engine/component/transform.h"
#include "src/util/math_util.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <assimp/scene.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#define PRT3_MODEL_EXT "p3m"
#define DOT_PRT3_MODEL_EXT ".p3m"

namespace std {
    // thanks, Basile Starynkevitch!
    template<> struct hash<aiString> {
        size_t operator()(aiString const& str) const {
            static constexpr int A = 54059; /* a prime */
            static constexpr int B = 76963; /* another prime */
            // static constexpr int C = 86969; /* yet another prime */
            static constexpr int FIRSTH = 37; /* also prime */
            unsigned h = FIRSTH;
            char const *s = str.C_Str();
            while (*s) {
                h = (h * A) ^ (s[0] * B);
                s++;
            }
            return h; // or return h % C;
        }
    };
}

namespace prt3 {

class Model {
public:
    struct Mesh;
    struct MeshMaterial;
    struct Vertex;
    struct BonedVertex;
    struct BoneData;
    struct Bone;
    struct Animation;
    struct AnimatedVertex;
    struct AnimationKey;
    struct Channel;
    struct Node;

    Model() {}
    Model(char const * path);

    std::vector<Node>         const & nodes()              const { return m_nodes; };
    std::vector<Mesh>         const & meshes()             const { return m_meshes; };
    std::vector<Animation>    const & animations()         const { return m_animations; };
    std::vector<MeshMaterial> const & materials()          const { return m_materials; };
    std::vector<Vertex>       const & vertex_buffer()      const { return m_vertex_buffer; };
    std::vector<BoneData>     const & vertex_bone_buffer() const { return m_vertex_bone_buffer; };
    std::vector<uint32_t>     const & index_buffer()       const { return m_index_buffer; };
    std::vector<Bone>         const & bones()              const { return m_bones; };

    std::vector<Node>         & nodes()              { return m_nodes; };
    std::vector<Mesh>         & meshes()             { return m_meshes; };
    std::vector<Animation>    & animations()         { return m_animations; };
    std::vector<MeshMaterial> & materials()          { return m_materials; };
    std::vector<Vertex>       & vertex_buffer()      { return m_vertex_buffer; };
    std::vector<BoneData>     & vertex_bone_buffer() { return m_vertex_bone_buffer; };
    std::vector<uint32_t>     & index_buffer()       { return m_index_buffer; };
    std::vector<Bone>         & bones()              { return m_bones; };

    void sample_animation(
        uint32_t animation_index,
        float t,
        bool looping,
        glm::mat4 * transforms,
        Transform * local_transforms
    ) const;

    void blend_animation(
        uint32_t animation_index_a,
        float t_a,
        bool looping_a,
        uint32_t animation_index_b,
        float t_b,
        bool looping_b,
        float blend_factor,
        glm::mat4 * transforms,
        Transform * local_transforms
    ) const;

    int32_t get_animation_index(char const * name) const;
    int32_t get_animation_index(char const * name, int32_t default_index) const;
    int32_t get_number_of_bones() const { return m_bones.size(); }

    inline bool is_animated() const { return !m_animations.empty(); }

    inline bool valid() const { return m_valid; }

    std::string const & name() const { return m_name; };
    std::string const & path() const { return m_path; }
    void set_path(std::string path) { m_path = path; }

    void save_prt3model(char const * path) const
    { std::ofstream out{path, std::ios::binary}; save_prt3model(out, path); }

private:
    std::string m_name;
    std::string m_path;
    bool m_valid = true;

    std::vector<Node> m_nodes;
    std::vector<Mesh> m_meshes;
    std::vector<Animation> m_animations;

    std::vector<uint8_t> m_position_locations;
    std::vector<glm::vec3> m_position_keys;
    std::vector<uint8_t> m_rotation_locations;
    std::vector<glm::quat> m_rotation_keys;
    std::vector<uint8_t> m_scale_locations;
    std::vector<glm::vec3> m_scale_keys;

    std::vector<Channel> m_channels;
    std::vector<MeshMaterial> m_materials;
    std::vector<Vertex> m_vertex_buffer;
    std::vector<BoneData> m_vertex_bone_buffer;
    std::vector<uint32_t> m_index_buffer;
    std::vector<Bone> m_bones;
    std::vector<uint32_t> m_bone_to_node;

    // maps animation names to animations
    // TODO: replace with fixed string?
    std::unordered_map<std::string, int32_t> m_name_to_animation;
    std::unordered_map<std::string, int32_t> m_name_to_node;

    void calculate_tangent_space();
    std::string get_texture(aiMaterial & aiMat, aiTextureType type, char const * model_path);

    void load_with_assimp(char const * path);

    bool attempt_load_cached(char const * path);

    void save_prt3model(std::ofstream & out, char const * path) const;

    void load_prt3model(std::FILE * in);
    inline void load_prt3model(char const * path)
    { load_prt3model(std::fopen(path, "rb")); }

};

struct Model::Node {
    int32_t parent_index = -1;
    int32_t mesh_index = -1;
    std::vector<uint32_t> child_indices;
    int32_t bone_index = -1;
    int32_t channel_index = -1;
    Transform transform;
    Transform inherited_transform;
    std::string name;
};

struct Model::Bone {
    glm::mat4 offset_matrix;
    glm::mat4 inverse_mesh_transform;
};

struct Model::Mesh {
    uint32_t start_index;
    uint32_t num_indices;
    int32_t start_bone;
    int32_t num_bones;
    int32_t material_index;
    uint32_t node_index;

    std::string name;
};

struct Model::MeshMaterial {
    std::string name;
    glm::vec4 albedo{1.0f, 1.0f, 1.0f, 1.0f};
    float metallic = 0.0f;
    float roughness = 1.0f;
    float ao = 1.0f;
    float emissive = 0.0f;
    bool twosided = false;
    bool transparent = false;
    std::string albedo_map;
    std::string normal_map;
    std::string metallic_map;
    std::string roughness_map;
    std::string ambient_occlusion_map;
};

struct Model::Channel {
    uint16_t n_frames;
    uint16_t loc_start_index;
    uint32_t pos_start_index;
    uint32_t rot_start_index;
    uint32_t scale_start_index;
};

struct Model::Animation {
    float duration;
    float ticks_per_second;
    uint16_t start_index;
    uint16_t num_indices;
};

struct Model::BoneData {
    glm::uvec4 bone_ids = { 0, 0, 0, 0 };
    glm::vec4 bone_weights = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct Model::Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture_coordinate;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Model::BonedVertex {
    Vertex vertex_data;
    BoneData bone_data;
};

} // namespace prt3

#endif