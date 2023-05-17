#include "model.h"

#include "src/main/args.h"
#include "src/util/file_util.h"
#include "src/util/checksum.h"
#include "src/util/log.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cstring>
#include <fstream>
#include <cstdio>

using namespace prt3;

inline uint16_t extract_key_index(
    uint8_t const * locations,
    uint16_t logical_index) {
    uint16_t n_sum_blocks = logical_index / 256;

    uint16_t index = 0;
    if (n_sum_blocks > 0) {
        uint16_t u16_sum = 0;
        memcpy(&u16_sum, &locations[34 * n_sum_blocks - 2], 2);
        index += u16_sum;
    }

    uint16_t n_sum_bits = logical_index % 256 + 1;

    uint16_t n_u64s = (n_sum_bits + 63 )/ 64;
    for (uint16_t i = 0; i < n_u64s; ++i) {
        size_t loc_index = n_sum_blocks * 34 + i * 8;

        uint64_t n_bits = i + 1 < n_u64s ? 64 : ((n_sum_bits - 1) % 64 + 1);
        size_t n_u8s = (n_bits + 7) / 8;

        uint64_t count_u64 = 0;
        memcpy(&count_u64, &locations[loc_index], n_u8s);

        if (i + 1 == n_u64s) {
            count_u64 = count_u64 & (~(~1ull << (n_bits - 1)));
        }

        index += number_of_set_bits(count_u64);
    }
    return index;
}

Model::Model(char const * path)
 : m_path{path} {
    // auto start_time = std::chrono::high_resolution_clock::now();

    char const * slash = std::strrchr(path, '/');
    m_name = slash ? slash + 1 : 0;
    if (!deserialize_model()) {
        load_with_assimp();
        serialize_model();
    }

    // auto end_time = std::chrono::high_resolution_clock::now();

    // auto duration =
    //     std::chrono::duration_cast<std::chrono::milliseconds>
    //     (end_time-start_time);
    // PRT3LOG("load time: %llu ms\n", duration.count());
}

int32_t Model::get_animation_index(char const * name) const {
    if (m_name_to_animation.find(name) == m_name_to_animation.end()) {
        return -1;
    }
    return m_name_to_animation.find(name)->second;
}

void Model::sample_animation(
    uint32_t animation_index,
    float t,
    bool looping,
    glm::mat4 * transforms,
    Transform * local_transforms
) const {
    auto const & animation = m_animations[animation_index];

    for (auto const & mesh : m_meshes) {
        auto end_index = mesh.start_bone + mesh.num_bones;
        for (int32_t i = mesh.start_bone; i < end_index; ++i) {
            Bone const & bone = m_bones[i];
            transforms[i] = bone.offset_matrix;

            int32_t node_index = static_cast<int32_t>(m_bone_to_node[i]);

            while (node_index != -1) {
                Node const & node = m_nodes[node_index];

                glm::mat4 tform = node.transform.to_matrix();

                int32_t channel_index = node.channel_index;
                if (channel_index != -1) {
                    uint32_t channel_buf_index =
                        animation.start_index + channel_index;
                    auto const & channel_info = m_channels[channel_buf_index];
                    uint32_t pos_start_index = channel_info.pos_start_index;
                    uint32_t rot_start_index = channel_info.rot_start_index;
                    uint32_t scale_start_index = channel_info.scale_start_index;
                    uint32_t num_frames = channel_info.n_frames;
                    glm::vec3 const * pos_channel =
                        &m_position_keys[pos_start_index];
                    glm::quat const * rot_channel =
                        &m_rotation_keys[rot_start_index];
                    glm::vec3 const * scale_channel =
                        &m_scale_keys[scale_start_index];

                    uint16_t loc_start = channel_info.loc_start_index;
                    uint8_t const * pos_locs = &m_position_locations[loc_start];
                    uint8_t const * rot_locs = &m_rotation_locations[loc_start];
                    uint8_t const * scale_locs = &m_scale_locations[loc_start];

                    float duration = animation.duration / animation.ticks_per_second;
                    float clip_time = t / duration;

                    // calculate prev and next frame
                    float frac_frame = clip_time * num_frames;
                    uint32_t prev_frame = static_cast<uint32_t>(frac_frame);
                    float frac = frac_frame - prev_frame;
                    uint32_t next_frame = (prev_frame + 1);

                    if (looping) {
                        prev_frame = prev_frame % num_frames;
                        next_frame = next_frame % num_frames;
                    } else {
                        prev_frame = glm::min(prev_frame, num_frames - 1);
                        next_frame = glm::min(next_frame, num_frames - 1);
                    }

                    uint16_t pos_prev = extract_key_index(pos_locs, prev_frame);
                    uint16_t pos_next = extract_key_index(pos_locs, next_frame);

                    uint16_t rot_prev = extract_key_index(rot_locs, prev_frame);
                    uint16_t rot_next = extract_key_index(rot_locs, next_frame);

                    uint16_t scale_prev = extract_key_index(scale_locs, prev_frame);
                    uint16_t scale_next = extract_key_index(scale_locs, next_frame);

                    glm::vec3 const & prev_pos = pos_channel[pos_prev];
                    glm::vec3 const & next_pos = pos_channel[pos_next];

                    glm::quat const & prev_rot = rot_channel[rot_prev];
                    glm::quat const & next_rot = rot_channel[rot_next];

                    glm::vec3 const & prev_scale = scale_channel[scale_prev];
                    glm::vec3 const & next_scale = scale_channel[scale_next];

                    glm::vec3 pos = glm::lerp(prev_pos, next_pos, frac);
                    glm::quat rot = glm::slerp(prev_rot, next_rot, frac);
                    glm::vec3 scale = glm::lerp(prev_scale, next_scale, frac);
                    tform = glm::translate(pos) * glm::toMat4(rot) * glm::scale(scale);

                    if (node.bone_index != -1) {
                        local_transforms[node.bone_index].position = pos;
                        local_transforms[node.bone_index].rotation = rot;
                        local_transforms[node.bone_index].scale = scale;
                    }
                }

                transforms[i] = tform * transforms[i];
                node_index = node.parent_index;
            }
            transforms[i] = bone.inverse_mesh_transform * transforms[i];
        }
    }
}

void Model::blend_animation(
    uint32_t animation_index_a,
    float t_a,
    bool looping_a,
    uint32_t animation_index_b,
    float t_b,
    bool looping_b,
    float blend_factor,
    glm::mat4 * transforms,
    Transform * local_transforms
) const {
    auto const & animation_a = m_animations[animation_index_a];
    auto const & animation_b = m_animations[animation_index_b];

    for (auto const & mesh : m_meshes) {
        auto end_index = mesh.start_bone + mesh.num_bones;
        for (int32_t i = mesh.start_bone; i < end_index; ++i) {
            Bone const & bone = m_bones[i];
            transforms[i] = bone.offset_matrix;

            int32_t node_index = static_cast<int32_t>(m_bone_to_node[i]);

            while (node_index != -1) {
                Node const & node = m_nodes[node_index];

                glm::mat4 tform = node.transform.to_matrix();

                int32_t channel_index = node.channel_index;
                if (channel_index != -1) {
                    uint32_t channel_buf_index_a =
                        animation_a.start_index + channel_index;
                    auto const & channel_info_a = m_channels[channel_buf_index_a];
                    uint32_t pos_start_index_a = channel_info_a.pos_start_index;
                    uint32_t rot_start_index_a = channel_info_a.rot_start_index;
                    uint32_t scale_start_index_a = channel_info_a.scale_start_index;
                    uint32_t num_frames_a = channel_info_a.n_frames;
                    glm::vec3 const * pos_channel_a =
                        &m_position_keys[pos_start_index_a];
                    glm::quat const * rot_channel_a =
                        &m_rotation_keys[rot_start_index_a];
                    glm::vec3 const * scale_channel_a =
                        &m_scale_keys[scale_start_index_a];

                    uint16_t loc_start_a = channel_info_a.loc_start_index;
                    uint8_t const * pos_locs_a = &m_position_locations[loc_start_a];
                    uint8_t const * rot_locs_a = &m_rotation_locations[loc_start_a];
                    uint8_t const * scale_locs_a = &m_scale_locations[loc_start_a];

                    uint32_t channel_buf_index_b =
                        animation_b.start_index + channel_index;
                    auto const & channel_info_b = m_channels[channel_buf_index_b];
                    uint32_t pos_start_index_b = channel_info_b.pos_start_index;
                    uint32_t rot_start_index_b = channel_info_b.rot_start_index;
                    uint32_t scale_start_index_b = channel_info_b.scale_start_index;
                    uint32_t num_frames_b = channel_info_b.n_frames;
                    glm::vec3 const * pos_channel_b =
                        &m_position_keys[pos_start_index_b];
                    glm::quat const * rot_channel_b =
                        &m_rotation_keys[rot_start_index_b];
                    glm::vec3 const * scale_channel_b =
                        &m_scale_keys[scale_start_index_b];

                    uint16_t loc_start_b = channel_info_b.loc_start_index;
                    uint8_t const * pos_locs_b = &m_position_locations[loc_start_b];
                    uint8_t const * rot_locs_b = &m_rotation_locations[loc_start_b];
                    uint8_t const * scale_locs_b = &m_scale_locations[loc_start_b];

                    float duration_a = animation_a.duration / animation_a.ticks_per_second;
                    float clip_time_a = t_a / duration_a;

                    float duration_b = animation_b.duration / animation_b.ticks_per_second;
                    float clip_time_b = t_b / duration_b;

                    // calculate prev and next frame for clip A
                    float frac_frame_a = clip_time_a * num_frames_a;
                    uint32_t prev_frame_a = static_cast<uint32_t>(frac_frame_a);
                    float frac_a = frac_frame_a - prev_frame_a;
                    uint32_t next_frame_a = (prev_frame_a + 1);

                    if (looping_a) {
                        prev_frame_a = prev_frame_a % num_frames_a;
                        next_frame_a = next_frame_a % num_frames_a;
                    } else {
                        prev_frame_a = glm::min(prev_frame_a, num_frames_a - 1);
                        next_frame_a = glm::min(next_frame_a, num_frames_a - 1);
                    }

                    // calculate prev and next frame for clip B
                    float frac_frame_b = clip_time_b * num_frames_b;
                    uint32_t prev_frame_b = static_cast<uint32_t>(frac_frame_b);
                    float frac_b = frac_frame_b - prev_frame_b;
                    uint32_t next_frame_b = (prev_frame_b + 1);

                    if (looping_b) {
                        prev_frame_b = prev_frame_b % num_frames_b;
                        next_frame_b = next_frame_b % num_frames_b;
                    } else {
                        prev_frame_b = glm::min(prev_frame_b, num_frames_b - 1);
                        next_frame_b = glm::min(next_frame_b, num_frames_b - 1);
                    }

                    uint16_t pos_prev_a = extract_key_index(pos_locs_a, prev_frame_a);
                    uint16_t pos_next_a = extract_key_index(pos_locs_a, next_frame_a);

                    uint16_t rot_prev_a = extract_key_index(rot_locs_a, prev_frame_a);
                    uint16_t rot_next_a = extract_key_index(rot_locs_a, next_frame_a);

                    uint16_t scale_prev_a = extract_key_index(scale_locs_a, prev_frame_a);
                    uint16_t scale_next_a = extract_key_index(scale_locs_a, next_frame_a);

                    uint16_t pos_prev_b = extract_key_index(pos_locs_b, prev_frame_b);
                    uint16_t pos_next_b = extract_key_index(pos_locs_b, next_frame_b);

                    uint16_t rot_prev_b = extract_key_index(rot_locs_b, prev_frame_b);
                    uint16_t rot_next_b = extract_key_index(rot_locs_b, next_frame_b);

                    uint16_t scale_prev_b = extract_key_index(scale_locs_b, prev_frame_b);
                    uint16_t scale_next_b = extract_key_index(scale_locs_b, next_frame_b);

                    // clip A
                    glm::vec3 const & prev_pos_a = pos_channel_a[pos_prev_a];
                    glm::vec3 const & next_pos_a = pos_channel_a[pos_next_a];

                    glm::quat const & prev_rot_a = rot_channel_a[rot_prev_a];
                    glm::quat const & next_rot_a = rot_channel_a[rot_next_a];

                    glm::vec3 const & prev_scale_a = scale_channel_a[scale_prev_a];
                    glm::vec3 const & next_scale_a = scale_channel_a[scale_next_a];

                    glm::vec3 pos_a = glm::lerp(prev_pos_a, next_pos_a, frac_a);
                    glm::quat rot_a = glm::slerp(prev_rot_a, next_rot_a, frac_a);
                    glm::vec3 scale_a = glm::lerp(prev_scale_a, next_scale_a, frac_a);

                    // clip B
                    glm::vec3 const & prev_pos_b = pos_channel_b[pos_prev_b];
                    glm::vec3 const & next_pos_b = pos_channel_b[pos_next_b];

                    glm::quat const & prev_rot_b = rot_channel_b[rot_prev_b];
                    glm::quat const & next_rot_b = rot_channel_b[rot_next_b];

                    glm::vec3 const & prev_scale_b = scale_channel_b[scale_prev_b];
                    glm::vec3 const & next_scale_b = scale_channel_b[scale_next_b];

                    glm::vec3 pos_b = glm::lerp(prev_pos_b, next_pos_b, frac_b);
                    glm::quat rot_b = glm::slerp(prev_rot_b, next_rot_b, frac_b);
                    glm::vec3 scale_b = glm::lerp(prev_scale_b, next_scale_b, frac_b);

                    // blend
                    glm::vec3 pos = glm::lerp(pos_a, pos_b, blend_factor);
                    glm::quat rot = glm::slerp(rot_a, rot_b, blend_factor);
                    glm::vec3 scale = glm::lerp(scale_a, scale_b, blend_factor);

                    tform = glm::translate(pos) * glm::toMat4(rot) * glm::scale(scale);

                    if (node.bone_index != -1) {
                        local_transforms[node.bone_index].position = pos;
                        local_transforms[node.bone_index].rotation = rot;
                        local_transforms[node.bone_index].scale = scale;
                    }
                }

                transforms[i] = tform * transforms[i];
                node_index = node.parent_index;
            }
            transforms[i] = bone.inverse_mesh_transform * transforms[i];
        }
    }
}

std::string Model::get_texture(aiMaterial & ai_mat, aiTextureType type, char const * model_path) {
    // TODO: optimize, too many temporary strings
    thread_local aiString ai_path;
    thread_local std::string tex_path;
    tex_path.clear();
    if (ai_mat.GetTexture(type, 0, &ai_path) == AI_SUCCESS) {
        thread_local std::string rel_path;
        rel_path = std::string(ai_path.C_Str());
        thread_local std::string model_path_str;
        model_path_str = std::string(model_path);

        tex_path = model_path_str.substr(0, model_path_str.rfind('/') + 1) + rel_path;
    }
    return tex_path;
}

void Model::calculate_tangent_space() {
    for (size_t i = 0; i < m_index_buffer.size(); i+=3) {
        auto & v0 = m_vertex_buffer[m_index_buffer[i]];
        auto & v1 = m_vertex_buffer[m_index_buffer[i+1]];
        auto & v2 = m_vertex_buffer[m_index_buffer[i+2]];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;
        glm::vec2 deltaUV1 = v1.texture_coordinate - v0.texture_coordinate;
        glm::vec2 deltaUV2 = v2.texture_coordinate - v0.texture_coordinate;

        glm::vec3 tan;
        glm::vec3 bi;

        float denom = (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
        if (denom == 0) {
            continue;
        }
        float f = 1.0f / denom;

        tan.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tan.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tan.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        // tan = glm::normalize(tan);
        v0.tangent += tan;
        v1.tangent += tan;
        v2.tangent += tan;

        bi.x = f * ((-deltaUV2.x * edge1.x) + deltaUV1.x * edge2.x);
        bi.y = f * ((-deltaUV2.x * edge1.y) + deltaUV1.x * edge2.y);
        bi.z = f * ((-deltaUV2.x * edge1.z) + deltaUV1.x * edge2.z);
        // bi = glm::normalize(bi);
        v0.bitangent += bi;
        v1.bitangent += bi;
        v2.bitangent += bi;
    }
    for (auto & vert : m_vertex_buffer) {
        if (glm::length(vert.tangent) == 0.0f) continue;
        vert.tangent = normalize(vert.tangent);
        vert.tangent = glm::normalize(vert.tangent - (vert.normal * glm::dot(vert.normal, vert.tangent)));
        glm::vec3 c = glm::cross(vert.normal, vert.tangent);
        if (glm::dot(c, vert.bitangent) < 0) {
            vert.tangent = -vert.tangent;
        }
        vert.bitangent = glm::cross(vert.normal, vert.tangent);
    }
}

void Model::load_with_assimp() {
    char const * path = m_path.c_str();

    Assimp::Importer importer;
    importer.SetPropertyInteger(
        AI_CONFIG_PP_SBP_REMOVE,
        aiPrimitiveType_LINE | aiPrimitiveType_POINT);

    aiScene const * scene = importer.ReadFile(
        path,
        aiProcess_ValidateDataStructure    |
        aiProcess_CalcTangentSpace         |
        aiProcess_Triangulate              |
        aiProcess_FlipUVs                  |
        aiProcess_FindDegenerates          |
        aiProcess_JoinIdenticalVertices    |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_ImproveCacheLocality     |
        aiProcess_SortByPType              |
        aiProcess_PopulateArmatureData
    );

    // check if import failed
    if(!scene) {
        PRT3ERROR("%s\n", importer.GetErrorString());
        m_valid = false;
        return;
        // assert(false && "failed to load file!");
    }

    // parse materials
    m_materials.resize(scene->mNumMaterials);
    for (size_t i = 0; i < m_materials.size(); ++i) {
        static aiString matName;
        aiGetMaterialString(scene->mMaterials[i], AI_MATKEY_NAME, &matName);
        m_materials[i].name = matName.C_Str();

        aiColor3D color;
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_AMBIENT, m_materials[i].ao);
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_EMISSIVE, m_materials[i].emissive);

        m_materials[i].albedo = { color.r, color.g, color.b, 1.0f };

        scene->mMaterials[i]->Get(AI_MATKEY_OPACITY, m_materials[i].albedo.a);
        scene->mMaterials[i]->Get(AI_MATKEY_TWOSIDED, m_materials[i].twosided);

        m_materials[i].albedo_map = get_texture(*scene->mMaterials[i], aiTextureType_DIFFUSE, path);
        m_materials[i].normal_map = get_texture(*scene->mMaterials[i], aiTextureType_NORMALS, path);
        m_materials[i].metallic_map = get_texture(*scene->mMaterials[i], aiTextureType_METALNESS, path);
        m_materials[i].roughness_map = get_texture(*scene->mMaterials[i], aiTextureType_SHININESS, path);
        m_materials[i].ambient_occlusion_map = get_texture(*scene->mMaterials[i], aiTextureType_AMBIENT, path);
    }

    /* Process node hierarchy */
    struct TFormNode {
        aiNode const * node;
        aiMatrix4x4 tform;
        int32_t parent_index;
    };
    thread_local std::vector<std::string> bone_to_name;
    bone_to_name.clear();

    thread_local std::vector<TFormNode> tform_nodes;

    tform_nodes.push_back({scene->mRootNode, scene->mRootNode->mTransformation, -1});
    while (!tform_nodes.empty()) {
        aiNode const * node = tform_nodes.back().node;
        int32_t parent_index = tform_nodes.back().parent_index;
        aiMatrix4x4 tform = tform_nodes.back().tform;
        tform_nodes.pop_back();

        // add node member
        int32_t node_index = m_nodes.size();
        m_nodes.push_back({});
        Node & n = m_nodes.back();
        n.name = node->mName.C_Str();

        glm::mat4 temp_tform;
        glm::mat4 temp_inherited_tform;
        std::memcpy(&temp_tform, &node->mTransformation, sizeof(glm::mat4));
        std::memcpy(&temp_inherited_tform, &tform, sizeof(glm::mat4));
        // assimp row-major, glm col-major
        n.transform.from_matrix(glm::transpose(temp_tform));
        n.inherited_transform.from_matrix(glm::transpose(temp_inherited_tform));

        m_name_to_node.insert({node->mName.C_Str(), node_index});

        n.parent_index = parent_index;
        if (n.parent_index != -1) {
           m_nodes[n.parent_index].child_indices.push_back(node_index);
        }

        // process all the node's meshes (if any)
        for(size_t i = 0; i < node->mNumMeshes; ++i) {
            aiMesh const * aiMesh = scene->mMeshes[node->mMeshes[i]];
            if (aiMesh->mNumFaces == 0) continue;

            // resize vertex buffer
            size_t prev_vertex_size = m_vertex_buffer.size();
            m_vertex_buffer.resize(prev_vertex_size + aiMesh->mNumVertices);
            // parse mesh
            unsigned int mesh_index = m_meshes.size();
            assert(n.mesh_index == -1 && "Multiple meshes per node not yet implemented!");
            n.mesh_index = mesh_index;

            m_meshes.push_back({});
            Mesh & mesh = m_meshes.back();
            mesh.name = aiMesh->mName.C_Str();
            mesh.material_index = aiMesh->mMaterialIndex;
            mesh.node_index = static_cast<uint32_t>(node_index);

            size_t vert = prev_vertex_size;
            bool has_texture_coordinates = aiMesh->HasTextureCoords(0);

            for (size_t j = 0; j < aiMesh->mNumVertices; ++j) {
                aiVector3D const & pos = aiMesh->mVertices[j];
                m_vertex_buffer[vert].position.x = pos.x;
                m_vertex_buffer[vert].position.y = pos.y;
                m_vertex_buffer[vert].position.z = pos.z;

                aiVector3D const & norm = aiMesh->mNormals[j];
                m_vertex_buffer[vert].normal.x = norm.x;
                m_vertex_buffer[vert].normal.y = norm.y;
                m_vertex_buffer[vert].normal.z = norm.z;

                aiVector3D const & tan = aiMesh->mTangents[j];
                m_vertex_buffer[vert].tangent.x = tan.x;
                m_vertex_buffer[vert].tangent.y = tan.y;
                m_vertex_buffer[vert].tangent.z = tan.z;

                aiVector3D const & bitan = aiMesh->mBitangents[j];
                m_vertex_buffer[vert].bitangent.x = bitan.x;
                m_vertex_buffer[vert].bitangent.y = bitan.y;
                m_vertex_buffer[vert].bitangent.z = bitan.z;

                if (has_texture_coordinates) {
                    m_vertex_buffer[vert].texture_coordinate.x =
                        aiMesh->mTextureCoords[0][j].x;
                    m_vertex_buffer[vert].texture_coordinate.y =
                        aiMesh->mTextureCoords[0][j].y;
                }
                ++vert;
            }
            size_t prev_index_size = m_index_buffer.size();
            m_index_buffer.resize(prev_index_size + 3 * aiMesh->mNumFaces);
            mesh.start_index = prev_index_size;
            mesh.num_indices = 3 * aiMesh->mNumFaces;
            size_t ind = prev_index_size;
            for (size_t j = 0; j < aiMesh->mNumFaces; ++j) {
                m_index_buffer[ind++] =
                    prev_vertex_size + aiMesh->mFaces[j].mIndices[0];
                m_index_buffer[ind++] =
                    prev_vertex_size + aiMesh->mFaces[j].mIndices[1];
                m_index_buffer[ind++] =
                    prev_vertex_size + aiMesh->mFaces[j].mIndices[2];
            }

            // bones
            m_vertex_bone_buffer.resize(m_vertex_buffer.size());

            int32_t bi = m_bones.size();
            m_bones.resize(m_bones.size() + aiMesh->mNumBones);
            bone_to_name.resize(m_bones.size() + aiMesh->mNumBones);

            if (aiMesh->mNumBones > 0) {
                mesh.start_bone = bi;
            }
            mesh.num_bones = aiMesh->mNumBones;

            for (size_t j = 0; j < aiMesh->mNumBones; ++j) {
                aiBone const * bone = aiMesh->mBones[j];

                thread_local std::string bone_name;
                bone_name = bone->mName.C_Str();

                bone_to_name[bi] = bone_name;

                std::memcpy(&m_bones[bi].offset_matrix,
                    &bone->mOffsetMatrix, sizeof(glm::mat4));

                // assimp row-major, glm col-major
                m_bones[bi].offset_matrix =
                    glm::transpose(m_bones[bi].offset_matrix);

                m_bones[bi].inverse_mesh_transform =
                    glm::inverse(n.inherited_transform.to_matrix());

                // store the bone weights and IDs in vertices
                for (size_t iv = 0; iv < bone->mNumWeights; ++iv) {
                    BoneData & bd =
                        m_vertex_bone_buffer[prev_vertex_size +
                                             bone->mWeights[iv].mVertexId];
                    // only store the 4 most influential bones
                    int least_index = -1;
                    float weight = bone->mWeights[iv].mWeight;
                    float least_weight = weight;
                    for (size_t ind = 0; ind < 4; ++ind) {
                        if (bd.bone_weights[ind] < least_weight) {
                            least_weight = bd.bone_weights[ind];
                            least_index = ind;
                        }
                    }
                    if (least_index != -1) {
                        bd.bone_ids[least_index] = bi;
                        bd.bone_weights[least_index] = weight;
                    }
                }

                ++bi;
            }
        }

        // process all children of the node
        for (size_t i = 0; i < node->mNumChildren; ++i) {
            tform_nodes.push_back({node->mChildren[i],
                                   node->mChildren[i]->mTransformation * tform,
                                   node_index});
        }
    }

    // parse animations
    m_animations.resize(scene->mNumAnimations);
    for (size_t i = 0; i < scene->mNumAnimations; ++i) {
        aiAnimation const * aiAnim = scene->mAnimations[i];

        // trim names such as "armature|<animationName>"
        char const * to_copy = strchr(aiAnim->mName.C_Str(), '|');
        if (to_copy != nullptr) {
            char name_buf[256];
            ++to_copy;
            strcpy(name_buf, to_copy);
            if (m_name_to_animation.find(name_buf) != m_name_to_animation.end()) {
                // TODO: Better resolution for this issue.
                m_name_to_animation.insert({aiAnim->mName.C_Str(), i});
            } else {
                m_name_to_animation.insert({name_buf, i});
            }
        } else {
            m_name_to_animation.insert({aiAnim->mName.C_Str(), i});
        }

        Animation & anim = m_animations[i];
        anim.duration = aiAnim->mDuration;
        anim.ticks_per_second = aiAnim->mTicksPerSecond;
        size_t n_channels = aiAnim->mNumChannels;
        size_t n_channels_prev = m_channels.size();
        m_channels.resize(m_channels.size() + n_channels);
        anim.start_index = n_channels_prev;
        anim.num_indices = n_channels;

        Channel * channels = &m_channels[n_channels_prev];

        for (size_t j = 0; j < n_channels; ++j) {
            aiNodeAnim const * aiChannel = aiAnim->mChannels[j];

            assert(m_name_to_node.find(aiChannel->mNodeName.C_Str())
                != m_name_to_node.end() &&
                "animation does not correspond to node");

            auto node_index =
                m_name_to_node.find(aiChannel->mNodeName.C_Str())->second;
            m_nodes[node_index].channel_index = j;

            assert(aiChannel->mNumPositionKeys == aiChannel->mNumRotationKeys &&
                aiChannel->mNumPositionKeys == aiChannel->mNumScalingKeys &&
                    "number of position, rotation and scaling keys need to match");

            size_t n_keys = aiChannel->mNumPositionKeys;

            channels[j].n_frames = n_keys;
            channels[j].loc_start_index = m_position_locations.size();
            channels[j].pos_start_index = m_position_keys.size();
            channels[j].rot_start_index = m_rotation_keys.size();
            channels[j].scale_start_index = m_scale_keys.size();

            size_t loc_start = m_position_locations.size();
            size_t n_byte_bytes = 2 * (n_keys / 256);
            size_t n_bit_bytes = (n_keys + 7) / 8;
            m_position_locations.resize(loc_start + n_byte_bytes + n_bit_bytes);
            m_rotation_locations.resize(loc_start + n_byte_bytes + n_bit_bytes);
            m_scale_locations.resize(loc_start + n_byte_bytes + n_bit_bytes);

            uint8_t * pos_loc = &m_position_locations[loc_start];
            uint8_t * rot_loc = &m_rotation_locations[loc_start];
            uint8_t * scale_loc = &m_scale_locations[loc_start];

            size_t pos_key_start = m_position_keys.size();
            size_t rot_key_start = m_rotation_keys.size();
            size_t scale_key_start = m_scale_keys.size();

            constexpr float qnan = std::numeric_limits<float>::quiet_NaN();
            aiVector3D last_pos = aiVector3D{qnan};
            aiQuaternion last_rot = aiQuaternion{qnan, qnan, qnan, qnan};
            aiVector3D last_scale = aiVector3D{qnan};
            for (size_t k = 0; k < n_keys; ++k) {
                aiVector3D const & ai_pos = aiChannel->mPositionKeys[k].mValue;
                aiQuaternion const & ai_rot = aiChannel->mRotationKeys[k].mValue;
                aiVector3D const & ai_scale = aiChannel->mScalingKeys[k].mValue;

                if (k > 0 && k % 256 == 0) {
                    uint16_t pos_u16_sum = static_cast<uint16_t>(
                        m_position_keys.size() - pos_key_start
                    );
                    uint16_t rot_u16_sum = static_cast<uint16_t>(
                        m_rotation_keys.size() - rot_key_start
                    );
                    uint16_t scale_u16_sum = static_cast<uint16_t>(
                        m_scale_keys.size() - scale_key_start
                    );
                    size_t byte_index = 34 * (k / 256) - 2;
                    memcpy(&pos_loc[byte_index], &pos_u16_sum, sizeof(uint16_t));
                    memcpy(&rot_loc[byte_index], &rot_u16_sum, sizeof(uint16_t));
                    memcpy(&scale_loc[byte_index], &scale_u16_sum, sizeof(uint16_t));
                }

                size_t bit_index = k % 256 + (k / 256) * 16;

                if (!ai_pos.Equal(last_pos, 0.001f)) {
                    m_position_keys.emplace_back(
                        glm::vec3{ ai_pos.x, ai_pos.y, ai_pos.z }
                    );
                    if (k % 256 != 0) {
                        set_nth_bit(pos_loc, bit_index);
                    }
                    last_pos = ai_pos;
                }

                if (!ai_rot.Equal(last_rot, 0.001f)) {
                    m_rotation_keys.emplace_back(
                        glm::quat{ ai_rot.w, ai_rot.x, ai_rot.y, ai_rot.z }
                    );
                    if (k % 256 != 0) {
                        set_nth_bit(rot_loc, bit_index);
                    }
                    last_rot = ai_rot;
                }

                if (!ai_scale.Equal(last_scale, 0.001f)) {
                    m_scale_keys.emplace_back(
                        glm::vec3{ ai_scale.x, ai_scale.y, ai_scale.z }
                    );
                    if (k % 256 != 0) {
                        set_nth_bit(scale_loc, bit_index);
                    }
                    last_scale = ai_scale;
                }
            }
        }
    }

    // set node Indices
    m_bone_to_node.resize(m_bones.size());
    for (size_t i = 0; i < m_bones.size(); ++i) {
        std::string const & bone_name = bone_to_name[i];

        assert(m_name_to_node.find(bone_name) != m_name_to_node.end() &&
               "No corresponding node for bone");
        size_t node_index = m_name_to_node.at(bone_name.c_str());
        assert(m_nodes[node_index].bone_index == -1);
        m_nodes[node_index].bone_index = i;

        m_bone_to_node[i] = static_cast<uint32_t>(node_index);
    }
    // calculate_tangent_space();
}

static std::string const serialized_postfix = "_prt3cache";

void Model::serialize_model() {
    std::string serialized_path = m_path + serialized_postfix;

    CRC32String checksum = compute_crc32(m_path.c_str());

    std::ofstream out(serialized_path, std::ios::binary);

    out.write(checksum.data(), checksum.writeable_size());

    write_stream(out, m_valid);

    write_stream(out, m_nodes.size());
    for (Node const & node : m_nodes) {
        write_stream(out, node.parent_index);
        write_stream(out, node.mesh_index);
        write_stream(out, node.channel_index);
        write_stream(out, node.bone_index);
        out << node.transform;
        out << node.inherited_transform;

        write_string(out, node.name);

        write_stream(out, node.child_indices.size());
        for (uint32_t child_index : node.child_indices) {
            write_stream(out, child_index);
        }
    }

    write_stream(out, m_meshes.size());
    for (Mesh const & mesh : m_meshes) {
        write_stream(out, mesh.start_index);
        write_stream(out, mesh.num_indices);
        write_stream(out, mesh.start_bone);
        write_stream(out, mesh.num_bones);
        write_stream(out, mesh.material_index);
        write_stream(out, mesh.node_index);

        write_string(out, mesh.name);
    }

    write_stream(out, m_animations.size());
    write_stream_n(out, m_animations.data(), m_animations.size());

    write_stream(out, m_channels.size());
    out.write(
        reinterpret_cast<char*>(m_channels.data()),
        m_channels.size() * sizeof(m_channels[0])
    );

    write_stream(out, m_position_keys.size());
    write_stream_n(out, m_position_keys.data(), m_position_keys.size());
    write_stream(out, m_position_locations.size());
    write_stream_n(out, m_position_locations.data(), m_position_locations.size());

    write_stream(out, m_rotation_keys.size());
    write_stream_n(out, m_rotation_keys.data(), m_rotation_keys.size());
    write_stream(out, m_rotation_locations.size());
    write_stream_n(out, m_rotation_locations.data(), m_rotation_locations.size());

    write_stream(out, m_scale_keys.size());
    write_stream_n(out, m_scale_keys.data(), m_scale_keys.size());
    write_stream(out, m_scale_locations.size());
    write_stream_n(out, m_scale_locations.data(), m_scale_locations.size());

    thread_local std::vector<std::string const *> animation_names;
    assert(m_animations.size() == m_name_to_animation.size() && "sizes differ");
    animation_names.resize(m_animations.size());

    for (auto const & pair : m_name_to_animation) {
        animation_names[pair.second] = &pair.first;
    }

    for (std::string const * name : animation_names) {
        write_string(out, *name);
    }

    write_stream(out, m_materials.size());
    for (Material const & material : m_materials) {
        write_stream(out, material.name.length());
        out.write(material.name.c_str(), material.name.length());

        write_stream(out, material.albedo);
        write_stream(out, material.metallic);
        write_stream(out, material.roughness);
        write_stream(out, material.ao);
        write_stream(out, material.emissive);
        write_stream(out, material.twosided);

        write_string(out, material.albedo_map);
        write_string(out, material.normal_map);
        write_string(out, material.metallic_map);
        write_string(out, material.roughness_map);
        write_string(out, material.ambient_occlusion_map);
    }

    write_stream(out, m_vertex_buffer.size());
    out.write(
        reinterpret_cast<char*>(m_vertex_buffer.data()),
        m_vertex_buffer.size() * sizeof(m_vertex_buffer[0])
    );

    write_stream(out, m_vertex_bone_buffer.size());
    out.write(
        reinterpret_cast<char*>(m_vertex_bone_buffer.data()),
        m_vertex_bone_buffer.size() * sizeof(m_vertex_bone_buffer[0])
    );

    write_stream(out, m_index_buffer.size());
    out.write(
        reinterpret_cast<char*>(m_index_buffer.data()),
        m_index_buffer.size() * sizeof(m_index_buffer[0])
    );

    write_stream(out, m_bones.size());
    out.write(
        reinterpret_cast<char*>(m_bones.data()),
        m_bones.size() * sizeof(m_bones[0])
    );

    for (uint32_t const & node_index : m_bone_to_node) {
        write_stream(out, node_index);
    }

    out.close();

#ifdef __EMSCRIPTEN__
    emscripten_save_file_via_put(serialized_path);
#endif // __EMSCRIPTEN__
}

bool Model::deserialize_model() {
    thread_local std::string serialized_path;
    serialized_path = m_path + serialized_postfix;

    std::FILE * in;

    in = std::fopen(serialized_path.c_str(), "rb");

    if (!in) {
        return false;
    }

    CRC32String checksum;
    std::fread(checksum.data(), 1, checksum.writeable_size(), in);

    if (!Args::force_cached()) {
        CRC32String current_checksum = compute_crc32(m_path.c_str());

        if (checksum != current_checksum) {
            return false;
        }
    }

    read_stream(in, m_valid);

    size_t n_nodes;
    read_stream(in, n_nodes);
    m_nodes.resize(n_nodes);
    for (Node & node : m_nodes) {
        read_stream(in, node.parent_index);
        read_stream(in, node.mesh_index);
        read_stream(in, node.channel_index);
        read_stream(in, node.bone_index);

        read_stream(in, node.transform.rotation);
        read_stream(in, node.transform.position);
        read_stream(in, node.transform.scale);

        read_stream(in, node.inherited_transform.rotation);
        read_stream(in, node.inherited_transform.position);
        read_stream(in, node.inherited_transform.scale);

        read_string(in, node.name);

        size_t n_indices;
        read_stream(in, n_indices);
        node.child_indices.resize(n_indices);
        for (uint32_t & child_index : node.child_indices) {
            read_stream(in, child_index);
        }
    }

    int node_ind = 0;
    m_name_to_node.reserve(m_nodes.size());
    for (Node const & node : m_nodes) {
        m_name_to_node[node.name] = node_ind;
        ++node_ind;
    }

    size_t n_meshes;
    read_stream(in, n_meshes);
    m_meshes.resize(n_meshes);
    for (Mesh & mesh : m_meshes) {
        read_stream(in, mesh.start_index);
        read_stream(in, mesh.num_indices);
        read_stream(in, mesh.start_bone);
        read_stream(in, mesh.num_bones);
        read_stream(in, mesh.material_index);
        read_stream(in, mesh.node_index);

        read_string(in, mesh.name);
    }

    size_t n_animations;
    read_stream(in, n_animations);
    m_animations.resize(n_animations);
    read_stream_n(in, m_animations.data(), n_animations);

    size_t n_channels;
    read_stream(in, n_channels);
    m_channels.resize(n_channels);
    read_stream_n(in, m_channels.data(), n_channels);

    size_t n_pos_keys;
    read_stream(in, n_pos_keys);
    m_position_keys.resize(n_pos_keys);
    read_stream_n(in, m_position_keys.data(), n_pos_keys);
    size_t n_pos_locs;
    read_stream(in, n_pos_locs);
    m_position_locations.resize(n_pos_locs);
    read_stream_n(in, m_position_locations.data(), n_pos_locs);

    size_t n_rot_keys;
    read_stream(in, n_rot_keys);
    m_rotation_keys.resize(n_rot_keys);
    read_stream_n(in, m_rotation_keys.data(), n_rot_keys);
    size_t n_rot_locs;
    read_stream(in, n_rot_locs);
    m_rotation_locations.resize(n_rot_locs);
    read_stream_n(in, m_rotation_locations.data(), n_rot_locs);

    size_t n_scale_keys;
    read_stream(in, n_scale_keys);
    m_scale_keys.resize(n_scale_keys);
    read_stream_n(in, m_scale_keys.data(), n_scale_keys);
    size_t n_scale_locs;
    read_stream(in, n_scale_locs);
    m_scale_locations.resize(n_scale_locs);
    read_stream_n(in, m_scale_locations.data(), n_scale_locs);

    m_name_to_animation.reserve(m_animations.size());
    for (int i = 0; i < static_cast<int>(m_animations.size()); ++i) {
        thread_local std::string name;
        read_string(in, name);
        m_name_to_animation[name] = i;
    }

    size_t n_materials;
    read_stream(in, n_materials);
    m_materials.resize(n_materials);
    for (Material & material : m_materials) {
        read_string(in, material.name);

        read_stream(in, material.albedo);
        read_stream(in, material.metallic);
        read_stream(in, material.roughness);
        read_stream(in, material.ao);
        read_stream(in, material.emissive);
        read_stream(in, material.twosided);

        read_string(in, material.albedo_map);
        read_string(in, material.normal_map);
        read_string(in, material.metallic_map);
        read_string(in, material.roughness_map);
        read_string(in, material.ambient_occlusion_map);
    }

    size_t n_vertex_buffer;
    read_stream(in, n_vertex_buffer);
    m_vertex_buffer.resize(n_vertex_buffer);
    read_stream_n(in, m_vertex_buffer.data(), n_vertex_buffer);

    size_t n_vertex_bone_buffer;
    read_stream(in, n_vertex_bone_buffer);
    m_vertex_bone_buffer.resize(n_vertex_bone_buffer);
    read_stream_n(in, m_vertex_bone_buffer.data(), n_vertex_bone_buffer);

    size_t n_index_buffer;
    read_stream(in, n_index_buffer);
    m_index_buffer.resize(n_index_buffer);
    read_stream_n(in, m_index_buffer.data(), n_index_buffer);

    size_t n_bones;
    read_stream(in, n_bones);
    m_bones.resize(n_bones);
    m_bone_to_node.resize(n_bones);
    read_stream_n(in, m_bones.data(), n_bones);
    read_stream_n(in, m_bone_to_node.data(), n_bones);

    std::fclose(in);

    return true;
}
