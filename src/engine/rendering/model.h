#ifndef PRT3_MODEL_H
#define PRT3_MODEL_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <assimp/scene.h>

#include <string>
#include <vector>
#include <unordered_map>

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
    struct Material;
    struct Vertex;
    struct BonedVertex;
    struct BoneData;
    struct Bone;
    struct Animation;
    struct AnimatedVertex;
    struct AnimationKey;
    struct AnimationNode;
    struct Node;

    Model(char const * path);

    std::vector<Node>      const & nodes()              const { return m_nodes; };
    std::vector<Mesh>      const & meshes()             const { return m_meshes; };
    std::vector<Animation> const & animations()         const { return m_animations; };
    std::vector<Material>  const & materials()          const { return m_materials; };
    std::vector<Vertex>    const & vertex_buffer()      const { return m_vertex_buffer; };
    std::vector<BoneData>  const & vertex_bone_buffer() const { return m_vertex_bone_buffer; };
    std::vector<uint32_t>  const & index_buffer()       const { return m_index_buffer; };
    std::vector<Bone>      const & bones()              const { return m_bones; };

    // void sampleAnimation(AnimationClip & clip, glm::mat4 * transforms) const;
    // void blendAnimation(AnimationClip & clipA,
    //                     AnimationClip & clipB,
    //                     float blendFactor,
    //                     glm::mat4 * transforms) const;

    int get_animation_index(char const * name) const;
    int get_number_of_bones() const { return m_bones.size(); }
    int get_bone_index(char const * name) const;
    glm::mat4 get_bone_transform(int index) const;
    glm::mat4 get_bone_transform(char const * name) const;

    inline bool is_animated() const { return m_animated; }

    std::string const & get_name() const { return m_name; };

private:
    std::string m_name;
    bool m_animated = false;

    std::vector<Node> m_nodes;
    std::vector<Mesh> m_meshes;
    std::vector<Animation> m_animations;
    std::vector<Material> m_materials;
    std::vector<Vertex> m_vertex_buffer;
    std::vector<BoneData> m_vertex_bone_buffer;
    std::vector<uint32_t> m_index_buffer;
    std::vector<Bone> m_bones;

    // maps animation names to animations
    // TODO: replace with own string type
    std::unordered_map<std::string, int> m_name_to_animation;
    std::unordered_map<std::string, int> m_name_to_bone;
    std::unordered_map<std::string, int> m_name_to_node;

    void calculate_tangent_space();
    // int get_texture(aiMaterial &aiMat, aiTextureType type, const char * model_path);
};

struct Model::Node {
    int parent_index = -1;
    int mesh_index = -1;
    std::vector<unsigned int> child_indices;
    std::vector<int> bone_indices;
    int channel_index = -1;
    glm::mat4 transform;
    std::string name;
};

struct Model::Material {
    std::string name;
    glm::vec4 albedo{1.0f, 1.0f, 1.0f, 1.0f};
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;
    float emissive = 0.0f;
    int32_t albedoIndex = -1;
    int32_t metallicIndex = -1;
    int32_t roughnessIndex = -1;
    int32_t aoIndex = -1;
    int32_t normalIndex = -1;
    bool twosided = false;
};

struct Model::Bone {
    glm::mat4 offset_matrix;
    glm::mat4 mesh_transform;
};

struct Model::Mesh {
    size_t start_index;
    size_t num_indices;
    int32_t material_index = 0;
    std::string name;
};

struct Model::AnimationKey {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scaling;
};

struct Model::AnimationNode {
    std::vector<AnimationKey> keys;
};

struct Model::Animation {
    float duration;
    double ticks_per_second;
    std::vector<AnimationNode> channels;
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