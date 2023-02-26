#include "model.h"

#include "src/util/file_util.h"
#include "src/util/checksum.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <cstring>
#include <fstream>

using namespace prt3;

Model::Model(char const * path)
 : m_path{path} {
    char const * slash = std::strchr(path, '/');
    m_name = slash ? slash + 1 : 0;
    if (!deserialize_model()) {
        load_with_assimp();
        serialize_model();
    }
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
                    auto const & channel = animation.channels[channel_index];

                    float duration = animation.duration / animation.ticks_per_second;
                    float clip_time = t / duration;

                    // calculate prev and next frame
                    int num_frames = animation.channels[channel_index].keys.size();
                    float frac_frame = clip_time * num_frames;
                    int prev_frame = static_cast<int>(frac_frame);
                    float frac = frac_frame - prev_frame;
                    int next_frame = (prev_frame + 1);

                    if (looping) {
                        prev_frame = prev_frame % num_frames;
                        next_frame = next_frame % num_frames;
                    } else {
                        prev_frame = glm::min(prev_frame, num_frames - 1);
                        next_frame = glm::min(next_frame, num_frames - 1);
                    }

                    glm::vec3 const & prev_pos = channel.keys[prev_frame].position;
                    glm::vec3 const & next_pos = channel.keys[next_frame].position;

                    glm::quat const & prev_rot = channel.keys[prev_frame].rotation;
                    glm::quat const & next_rot = channel.keys[next_frame].rotation;

                    glm::vec3 const & prev_scale = channel.keys[prev_frame].scaling;
                    glm::vec3 const & next_scale = channel.keys[next_frame].scaling;

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
                    auto & channel_a = animation_a.channels[channel_index];
                    auto & channel_b = animation_b.channels[channel_index];

                    float duration_a = animation_a.duration / animation_a.ticks_per_second;
                    float clip_time_a = t_a / duration_a;

                    float duration_b = animation_b.duration / animation_b.ticks_per_second;
                    float clip_time_b = t_b / duration_b;

                    // calculate prev and next frame for clip A
                    int num_frames_a = animation_a.channels[channel_index].keys.size();
                    float frac_frame_a = clip_time_a * num_frames_a;
                    int prev_frame_a = static_cast<int>(frac_frame_a);
                    float frac_a = frac_frame_a - prev_frame_a;
                    int next_frame_a = (prev_frame_a + 1);

                    if (looping_a) {
                        prev_frame_a = prev_frame_a % num_frames_a;
                        next_frame_a = next_frame_a % num_frames_a;
                    } else {
                        prev_frame_a = glm::min(prev_frame_a, num_frames_a - 1);
                        next_frame_a = glm::min(next_frame_a, num_frames_a - 1);
                    }

                    // calculate prev and next frame for clip B
                    int num_frames_b = animation_b.channels[channel_index].keys.size();
                    float frac_frame_b = clip_time_b * num_frames_b;
                    int prev_frame_b = static_cast<int>(frac_frame_b);
                    float frac_b = frac_frame_b - prev_frame_b;
                    int next_frame_b = (prev_frame_b + 1);

                    if (looping_b) {
                        prev_frame_b = prev_frame_b % num_frames_b;
                        next_frame_b = next_frame_b % num_frames_b;
                    } else {
                        prev_frame_b = glm::min(prev_frame_b, num_frames_b - 1);
                        next_frame_b = glm::min(next_frame_b, num_frames_b - 1);
                    }

                    // clip A
                    glm::vec3 const & prev_pos_a = channel_a.keys[prev_frame_a].position;
                    glm::vec3 const & next_pos_a = channel_a.keys[next_frame_a].position;

                    glm::quat const & prev_rot_a = channel_a.keys[prev_frame_a].rotation;
                    glm::quat const & next_rot_a = channel_a.keys[next_frame_a].rotation;

                    glm::vec3 const & prev_scale_a = channel_a.keys[prev_frame_a].scaling;
                    glm::vec3 const & next_scale_a = channel_a.keys[next_frame_a].scaling;

                    glm::vec3 pos_a = glm::lerp(prev_pos_a, next_pos_a, frac_a);
                    glm::quat rot_a = glm::slerp(prev_rot_a, next_rot_a, frac_a);
                    glm::vec3 scale_a = glm::lerp(prev_scale_a, next_scale_a, frac_a);

                    // clip B
                    glm::vec3 const & prev_pos_b = channel_b.keys[prev_frame_b].position;
                    glm::vec3 const & next_pos_b = channel_b.keys[next_frame_b].position;

                    glm::quat const & prev_rot_b = channel_b.keys[prev_frame_b].rotation;
                    glm::quat const & next_rot_b = channel_b.keys[next_frame_b].rotation;

                    glm::vec3 const & prev_scale_b = channel_b.keys[prev_frame_b].scaling;
                    glm::vec3 const & next_scale_b = channel_b.keys[next_frame_b].scaling;

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
        std::cout << importer.GetErrorString() << std::endl;
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
            char nameBuf[256];
            ++to_copy;
            strcpy(nameBuf, to_copy);
            if (m_name_to_animation.find(nameBuf) != m_name_to_animation.end()) {
                // TODO: Better resolution for this issue.
                m_name_to_animation.insert({aiAnim->mName.C_Str(), i});
            } else {
                m_name_to_animation.insert({nameBuf, i});
            }
        } else {
            m_name_to_animation.insert({aiAnim->mName.C_Str(), i});
        }

        Animation & anim = m_animations[i];
        anim.duration = aiAnim->mDuration;
        anim.ticks_per_second = aiAnim->mTicksPerSecond;
        anim.channels.resize(aiAnim->mNumChannels);

        for (size_t j = 0; j < aiAnim->mNumChannels; ++j) {
            aiNodeAnim const * aiChannel = aiAnim->mChannels[j];
            AnimationNode & channel = anim.channels[j];

            assert(m_name_to_node.find(aiChannel->mNodeName.C_Str())
                != m_name_to_node.end() &&
                "animation does not correspond to node");

            auto node_index =
                m_name_to_node.find(aiChannel->mNodeName.C_Str())->second;
            m_nodes[node_index].channel_index = j;

            assert(aiChannel->mNumPositionKeys == aiChannel->mNumRotationKeys &&
                aiChannel->mNumPositionKeys == aiChannel->mNumScalingKeys &&
                    "number of position, rotation and scaling keys need to match");

            channel.keys.resize(aiChannel->mNumPositionKeys);

            for (size_t k = 0; k < channel.keys.size(); ++k) {
                aiVector3D const & aiPos = aiChannel->mPositionKeys[k].mValue;
                aiQuaternion const & aiRot = aiChannel->mRotationKeys[k].mValue;
                aiVector3D const & aiScale = aiChannel->mScalingKeys[k].mValue;
                channel.keys[k].position = { aiPos.x, aiPos.y, aiPos.z };
                channel.keys[k].rotation = { aiRot.w, aiRot.x, aiRot.y, aiRot.z };
                channel.keys[k].scaling = { aiScale.x, aiScale.y, aiScale.z };
            }
        }
    }

    // set node Indices
    m_bone_to_node.resize(m_bones.size());
    for (size_t i = 0; i < m_bones.size(); ++i) {
        std::string const & bone_name = bone_to_name[i];

        assert(m_name_to_node.find(bone_name) != m_name_to_node.end() &&
               "No corresponding node for bone");
        size_t node_index = m_name_to_node.at(bone_name);
        assert(m_nodes[node_index].bone_index == -1);
        m_nodes[node_index].bone_index = i;

        m_bone_to_node[i] = static_cast<uint32_t>(node_index);
    }

    // calculate_tangent_space();
}

static std::string const serialized_postfix = "_prt3cache";

void Model::serialize_model() {
    std::string serialized_path = m_path + serialized_postfix;

    MD5String checksum = compute_md5(m_path.c_str());

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
    for (Animation const & animation : m_animations) {
        write_stream(out, animation.duration);
        write_stream(out, animation.ticks_per_second);

        write_stream(out, animation.channels.size());
        for (AnimationNode const & channel : animation.channels) {
            write_stream(out, channel.keys.size());
            for (AnimationKey const & key : channel.keys) {
                write_stream(out, key.position);
                write_stream(out, key.rotation);
                write_stream(out, key.scaling);
            }
        }
    }
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
    for (Vertex const & vertex : m_vertex_buffer) {
        write_stream(out, vertex.position);
        write_stream(out, vertex.normal);
        write_stream(out, vertex.texture_coordinate);
        write_stream(out, vertex.tangent);
        write_stream(out, vertex.bitangent);
    }

    write_stream(out, m_vertex_bone_buffer.size());
    for (BoneData const & bone_data : m_vertex_bone_buffer) {
        write_stream(out, bone_data.bone_ids);
        write_stream(out, bone_data.bone_weights);
    }

    write_stream(out, m_index_buffer.size());
    for (uint32_t const & index : m_index_buffer) {
        write_stream(out, index);
    }

    write_stream(out, m_bones.size());
    for (Bone const & bone : m_bones) {
        write_stream(out, bone.offset_matrix);
        write_stream(out, bone.inverse_mesh_transform);

    }
    for (uint32_t const & node_index : m_bone_to_node) {
        write_stream(out, node_index);
    }

    out.close();

    emscripten_save_file_via_put(serialized_path);
}

bool Model::deserialize_model() {
    std::string serialized_path = m_path + serialized_postfix;

    std::ifstream in(serialized_path, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    MD5String current_checksum = compute_md5(m_path.c_str());

    MD5String checksum;
    in.read(checksum.data(), checksum.writeable_size());

    if (checksum != current_checksum) {
        return false;
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
        in >> node.transform;
        in >> node.inherited_transform;

        read_string(in, node.name);

        size_t n_indices;
        read_stream(in, n_indices);
        node.child_indices.resize(n_indices);
        for (uint32_t & child_index : node.child_indices) {
            read_stream(in, child_index);
        }
    }

    int node_ind = 0;
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
    for (Animation & animation : m_animations) {
        read_stream(in, animation.duration);
        read_stream(in, animation.ticks_per_second);

        size_t n_channels;
        read_stream(in, n_channels);
        animation.channels.resize(n_channels);
        for (AnimationNode & channel : animation.channels) {
            size_t n_keys;
            read_stream(in, n_keys);
            channel.keys.resize(n_keys);
            for (AnimationKey & key : channel.keys) {
                read_stream(in, key.position);
                read_stream(in, key.rotation);
                read_stream(in, key.scaling);
            }
        }
    }

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
    for (Vertex & vertex : m_vertex_buffer) {
        read_stream(in, vertex.position);
        read_stream(in, vertex.normal);
        read_stream(in, vertex.texture_coordinate);
        read_stream(in, vertex.tangent);
        read_stream(in, vertex.bitangent);
    }

    size_t n_vertex_bone_buffer;
    read_stream(in, n_vertex_bone_buffer);
    m_vertex_bone_buffer.resize(n_vertex_bone_buffer);
    for (BoneData & bone_data : m_vertex_bone_buffer) {
        read_stream(in, bone_data.bone_ids);
        read_stream(in, bone_data.bone_weights);
    }

    size_t n_index_buffer;
    read_stream(in, n_index_buffer);
    m_index_buffer.resize(n_index_buffer);
    for (uint32_t & index : m_index_buffer) {
        read_stream(in, index);
    }

    size_t n_bones;
    read_stream(in, n_bones);
    m_bones.resize(n_bones);
    m_bone_to_node.resize(n_bones);
    for (Bone & bone : m_bones) {
        read_stream(in, bone.offset_matrix);
        read_stream(in, bone.inverse_mesh_transform);
    }
    for (uint32_t & node_index : m_bone_to_node) {
        read_stream(in, node_index);
    }

    in.close();

    return true;
}
