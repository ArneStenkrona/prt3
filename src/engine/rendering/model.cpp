#include "model.h"


#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <cstring>

using namespace prt3;

Model::Model(char const * path) {
    m_animated = false; // TODO: allow animation import

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
                                aiPrimitiveType_LINE | aiPrimitiveType_POINT);

    aiScene const * scene = importer.ReadFile(path,
                                              aiProcess_CalcTangentSpace         |
                                              aiProcess_Triangulate              |
                                              aiProcess_FlipUVs                  |
                                              aiProcess_FindDegenerates          |
                                              aiProcess_JoinIdenticalVertices    |
                                              aiProcess_RemoveRedundantMaterials |
                                              aiProcess_ImproveCacheLocality     |
                                              aiProcess_SortByPType);

    // check if import failed
    if(!scene) {
        std::cout << importer.GetErrorString() << std::endl;
        assert(false && "failed to load file!");
    }

    m_name = std::strrchr(path, '/') + 1;

    // parse materials
    m_materials.resize(scene->mNumMaterials);
    for (size_t i = 0; i < m_materials.size(); ++i) {
 aiString matName;
        aiGetMaterialString(scene->mMaterials[i], AI_MATKEY_NAME, &matName);
        m_materials[i].name = matName.C_Str();

        aiColor3D color;
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_SPECULAR, m_materials[i].roughness);
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_AMBIENT, m_materials[i].ao);
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_EMISSIVE, m_materials[i].emissive);

        m_materials[i].albedo = { color.r, color.g, color.b, 1.0f };

        scene->mMaterials[i]->Get(AI_MATKEY_OPACITY, m_materials[i].albedo.a);
        scene->mMaterials[i]->Get(AI_MATKEY_TWOSIDED, m_materials[i].twosided);

        // m_materials[i].albedoIndex = get_texture(*scene->mMaterials[i], aiTextureType_DIFFUSE, path);
        // m_materials[i].metallicIndex = get_texture(*scene->mMaterials[i], aiTextureType_METALNESS, path);
        // m_materials[i].roughnessIndex = get_texture(*scene->mMaterials[i], aiTextureType_SHININESS, path);
        // m_materials[i].aoIndex = get_texture(*scene->mMaterials[i], aiTextureType_AMBIENT, path);
        // m_materials[i].normalIndex = get_texture(*scene->mMaterials[i], aiTextureType_NORMALS, path);
    }

    /* Process node hierarchy */
    struct TFormNode {
        aiNode* node;
        aiMatrix4x4 tform;
        int32_t parent_index;
    };
    std::vector<std::string> bone_to_name;

    std::vector<TFormNode> tform_nodes;
    tform_nodes.push_back({scene->mRootNode, scene->mRootNode->mTransformation, -1});
    while(!tform_nodes.empty()) {
        aiNode *node = tform_nodes.back().node;
        int32_t parent_index = tform_nodes.back().parent_index;
        aiMatrix4x4 tform = tform_nodes.back().tform;
        aiMatrix3x3 invtpos = aiMatrix3x3(tform);
        invtpos.Inverse().Transpose();
        tform_nodes.pop_back();

        // add node member
        int32_t node_index = m_nodes.size();
        m_nodes.push_back({});
        Node & n = m_nodes.back();
        n.name = node->mName.C_Str();
        std::memcpy(&n.transform, &node->mTransformation, sizeof(glm::mat4));
        // assimp row-major, glm col-major
        n.transform = glm::transpose(n.transform);
        m_name_to_node.insert({node->mName.C_Str(), node_index});

        n.parent_index = parent_index;
        if (n.parent_index != -1) {
           m_nodes[n.parent_index].child_indices.push_back(node_index);
        }

        // process all the node's meshes (if any)
        for(size_t i = 0; i < node->mNumMeshes; ++i) {
            aiMesh *aiMesh = scene->mMeshes[node->mMeshes[i]];
            if (aiMesh->mNumFaces == 0) continue;

            // resize vertex buffer
            size_t prev_vertex_size = m_vertex_buffer.size();
            m_vertex_buffer.resize(prev_vertex_size + aiMesh->mNumVertices);
            // parse mesh
            unsigned int mesh_index = m_meshes.size();
            assert(n.mesh_index == -1 && "Multiple meshes per node not yet implemented!");
            n.mesh_index = mesh_index;

            m_meshes.push_back({});
            Mesh &mesh = m_meshes.back();
            mesh.name = aiMesh->mName.C_Str();
            mesh.material_index = aiMesh->mMaterialIndex;

            size_t vert = prev_vertex_size;
            bool has_texture_coordinates = aiMesh->HasTextureCoords(0);
            for (size_t j = 0; j < aiMesh->mNumVertices; ++j) {
                aiVector3D pos = tform * aiMesh->mVertices[j];
                m_vertex_buffer[vert].position.x = pos.x;
                m_vertex_buffer[vert].position.y = pos.y;
                m_vertex_buffer[vert].position.z = pos.z;

                aiVector3D norm = (invtpos * aiMesh->mNormals[j]).Normalize();
                m_vertex_buffer[vert].normal.x = norm.x;
                m_vertex_buffer[vert].normal.y = norm.y;
                m_vertex_buffer[vert].normal.z = norm.z;

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
            if (m_animated) {
                m_vertex_bone_buffer.resize(m_vertex_buffer.size());
                size_t prevBoneSize = m_bones.size();
                m_bones.resize(prevBoneSize + aiMesh->mNumBones);
                bone_to_name.resize(prevBoneSize + aiMesh->mNumBones);

                for (size_t j = 0; j < aiMesh->mNumBones; ++j) {
                    size_t bi = prevBoneSize + j;
                    aiBone const * bone = aiMesh->mBones[j];

                    bone_to_name[bi] = bone->mName.C_Str();
                    m_name_to_bone[bone->mName.C_Str()] = bi;

                    std::memcpy(&m_bones[bi].offset_matrix,
                                &bone->mOffsetMatrix, sizeof(glm::mat4));

                    std::memcpy(&m_bones[bi].mesh_transform,
                                &node->mTransformation,
                                sizeof(glm::mat4));
                    m_bones[bi].mesh_transform =
                        glm::transpose(m_bones[bi].mesh_transform);
                    // assimp row-major, glm col-major
                    m_bones[bi].offset_matrix =
                        glm::transpose(m_bones[bi].offset_matrix) *
                        glm::inverse(m_bones[bi].mesh_transform);
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
                }
            }
        }

        // process all children of the node
        for (size_t i = 0; i < node->mNumChildren; ++i) {
            tform_nodes.push_back({node->mChildren[i],
                                   tform * node->mChildren[i]->mTransformation,
                                   node_index});
        }
    }
    // parse animations
    if (m_animated) {
        m_animations.resize(scene->mNumAnimations);
        for (size_t i = 0; i < scene->mNumAnimations; ++i) {
            aiAnimation const * aiAnim = scene->mAnimations[i];

            // trim names such as "armature|<animationName>"
            const char * toCopy = strchr(aiAnim->mName.C_Str(), '|');
            if (toCopy != nullptr) {
                char nameBuf[256];
                ++toCopy;
                strcpy(nameBuf, toCopy);
                m_name_to_animation.insert({nameBuf, i});
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
        for (size_t i = 0; i < m_bones.size(); ++i) {
            std::string const & bone_name = bone_to_name[i];

            assert(m_name_to_node.find(bone_name) != m_name_to_node.end() &&
                   "No corresponding node for bone");
            size_t node_index = m_name_to_node.find(bone_name)->second;
            m_nodes[node_index].bone_indices.push_back(i);
        }
    }

    calculate_tangent_space();
}

int Model::get_animation_index(char const * name) const {
    if (m_name_to_animation.find(name) == m_name_to_animation.end()) {
        return -1;
    }
    return m_name_to_animation.find(name)->second;
}

int Model::get_bone_index(char const * name) const {
    if (m_name_to_bone.find(name) == m_name_to_bone.end()) {
        return -1;
    }
    return m_name_to_bone.find(name)->second;
}

glm::mat4 Model::get_bone_transform(int index) const {
    return glm::inverse(m_bones[index].offset_matrix);
}


glm::mat4 Model::get_bone_transform(char const * name) const {
    if (m_name_to_node.find(name) == m_name_to_node.end()) {
        assert(false && "No bone by that name!");
    }

    int index = m_name_to_bone.find(name)->second;
    return get_bone_transform(index);
}

// void Model::sampleAnimation(AnimationClip & clip, glm::mat4 * transforms) const {
//     assert(mAnimated);
//     int animationIndex = getAnimationIndex(clip.m_clipName);
//     animationIndex = animationIndex == -1 ? 0 : animationIndex;

//     auto const & animation = animations[animationIndex];

//     struct IndexedTForm {
//         int32_t index;
//         glm::mat4 tform;
//     };

//     std::vector<IndexedTForm> nodeIndices;
//     nodeIndices.push_back({0, glm::mat4(1.0f)});
//     while (!nodeIndices.empty()) {
//         auto index = nodeIndices.back().index;
//         auto parentTForm = nodeIndices.back().tform;
//         nodeIndices.pop_back();

//         glm::mat4 tform = m_nodes[index].transform;
//         int32_t channelIndex = m_nodes[index].channelIndex;

//         if (channelIndex != -1) {
//             auto & channel = animation.channels[channelIndex];

//             float duration = animation.duration / animation.ticksPerSecond;
//             float clipTime = clip.m_time / duration;

//             // calculate prev and next frame
//             int numFrames = animation.channels[channelIndex].keys.size();
//             float fracFrame = clipTime * numFrames;
//             int prevFrame = static_cast<int>(fracFrame);
//             float frac = fracFrame - prevFrame;
//             int nextFrame = (prevFrame + 1);

//             if (clip.m_loop) {
//                 prevFrame = prevFrame % numFrames;
//                 nextFrame = nextFrame % numFrames;
//             } else {
//                 prevFrame = glm::min(prevFrame, numFrames - 1);
//                 nextFrame = glm::min(nextFrame, numFrames - 1);
//                 clip.m_completed = prevFrame == numFrames - 1;
//             }

//             glm::vec3 const & prevPos = channel.keys[prevFrame].position;
//             glm::vec3 const & nextPos = channel.keys[nextFrame].position;

//             glm::quat const & prevRot = channel.keys[prevFrame].rotation;
//             glm::quat const & nextRot = channel.keys[nextFrame].rotation;

//             glm::vec3 const & prevScale = channel.keys[prevFrame].scaling;
//             glm::vec3 const & nextScale = channel.keys[nextFrame].scaling;

//             glm::vec3 pos = glm::lerp(prevPos, nextPos, frac);
//             glm::quat rot = glm::slerp(prevRot, nextRot, frac);
//             glm::vec3 scale = glm::lerp(prevScale, nextScale, frac);
//             tform = glm::translate(pos) * glm::toMat4(rot) * glm::scale(scale);
//         }
//         // pose matrix
//         glm::mat4 poseMatrix = parentTForm * tform;

//         for (auto const & boneIndex : m_nodes[index].boneIndices) {
//             transforms[boneIndex] = poseMatrix * bones[boneIndex].offsetMatrix;
//         }

//         for (auto const & childIndex : m_nodes[index].child_indices) {
//             nodeIndices.push_back({childIndex, poseMatrix});
//         }
//     }
// }

// void Model::blendAnimation(AnimationClip & clipA,
//                            AnimationClip & clipB,
//                            float blendFactor,
//                            glm::mat4 * transforms) const {
//     assert(mAnimated);

//     int animationIndexA = getAnimationIndex(clipA.m_clipName);
//     animationIndexA = animationIndexA == -1 ? 0 : animationIndexA;
//     int animationIndexB = getAnimationIndex(clipB.m_clipName);
//     animationIndexB = animationIndexB == -1 ? 0 : animationIndexB;

//     auto const & animationA = animations[animationIndexA];
//     auto const & animationB = animations[animationIndexB];

//     struct IndexedTForm {
//         int index;
//         glm::mat4 tform;
//     };
//     std::vector<IndexedTForm> nodeIndices;
//     nodeIndices.push_back({0, glm::mat4(1.0f)});
//     while (!nodeIndices.empty()) {
//         auto index = nodeIndices.back().index;
//         auto parentTForm = nodeIndices.back().tform;
//         nodeIndices.pop_back();

//         glm::mat4 tform = m_nodes[index].transform;
//         int channelIndex = m_nodes[index].channelIndex;

//         if (channelIndex != -1) {
//             auto & channelA = animationA.channels[channelIndex];
//             auto & channelB = animationB.channels[channelIndex];

//             float durationA = animationA.duration / animationA.ticksPerSecond;
//             float clipTimeA = clipA.m_time / durationA;
//             // float clipTimeA = clipA.m_time * animationA.ticksPerSecond;

//             float durationB = animationB.duration / animationB.ticksPerSecond;
//             float clipTimeB = clipB.m_time / durationB;
//             // float clipTimeB = clipB.m_time * animationB.ticksPerSecond;

//             // calculate prev and next frame for clip A
//             int numFramesA = animationA.channels[channelIndex].keys.size();
//             float fracFrameA = clipTimeA * numFramesA;
//             int prevFrameA = static_cast<int>(fracFrameA);
//             float fracA = fracFrameA - prevFrameA;
//             int nextFrameA = (prevFrameA + 1);

//             if (clipA.m_loop) {
//                 prevFrameA = prevFrameA % numFramesA;
//                 nextFrameA = nextFrameA % numFramesA;
//             } else {
//                 prevFrameA = glm::min(prevFrameA, numFramesA - 1);
//                 nextFrameA = glm::min(nextFrameA, numFramesA - 1);
//                 clipA.m_completed = prevFrameA == numFramesA - 1;
//             }

//             // calculate prev and next frame for clip B
//             int numFramesB = animationB.channels[channelIndex].keys.size();
//             float fracFrameB = clipTimeB * numFramesB;
//             int prevFrameB = static_cast<int>(fracFrameB);
//             float fracB = fracFrameB - prevFrameB;
//             int nextFrameB = (prevFrameB + 1);

//             if (clipB.m_loop) {
//                 prevFrameB = prevFrameB % numFramesB;
//                 nextFrameB = nextFrameB % numFramesB;
//             } else {
//                 prevFrameB = glm::min(prevFrameB, numFramesB - 1);
//                 nextFrameB = glm::min(nextFrameB, numFramesB - 1);
//                 clipB.m_completed = prevFrameB == numFramesB - 1;
//             }

//             // clip A
//             glm::vec3 const & prevPosA = channelA.keys[prevFrameA].position;
//             glm::vec3 const & nextPosA = channelA.keys[nextFrameA].position;

//             glm::quat const & prevRotA = channelA.keys[prevFrameA].rotation;
//             glm::quat const & nextRotA = channelA.keys[nextFrameA].rotation;

//             glm::vec3 const & prevScaleA = channelA.keys[prevFrameA].scaling;
//             glm::vec3 const & nextScaleA = channelA.keys[nextFrameA].scaling;

//             glm::vec3 posA = glm::lerp(prevPosA, nextPosA, fracA);
//             glm::quat rotA = glm::slerp(prevRotA, nextRotA, fracA);
//             glm::vec3 scaleA = glm::lerp(prevScaleA, nextScaleA, fracA);

//             // clip B
//             glm::vec3 const & prevPosB = channelB.keys[prevFrameB].position;
//             glm::vec3 const & nextPosB = channelB.keys[nextFrameB].position;

//             glm::quat const & prevRotB = channelB.keys[prevFrameB].rotation;
//             glm::quat const & nextRotB = channelB.keys[nextFrameB].rotation;

//             glm::vec3 const & prevScaleB = channelB.keys[prevFrameB].scaling;
//             glm::vec3 const & nextScaleB = channelB.keys[nextFrameB].scaling;

//             glm::vec3 posB = glm::lerp(prevPosB, nextPosB, fracB);
//             glm::quat rotB = glm::slerp(prevRotB, nextRotB, fracB);
//             glm::vec3 scaleB = glm::lerp(prevScaleB, nextScaleB, fracB);

//             // blend
//             glm::vec3 pos = glm::lerp(posA, posB, blendFactor);
//             glm::quat rot = glm::slerp(rotA, rotB, blendFactor);
//             glm::vec3 scale = glm::lerp(scaleA, scaleB, blendFactor);

//             tform = glm::translate(pos) * glm::toMat4(rot) * glm::scale(scale);
//         }
//         // pose matrix
//         glm::mat4 poseMatrix = parentTForm * tform;

//         for (auto const & boneIndex : m_nodes[index].boneIndices) {
//             transforms[boneIndex] = poseMatrix * bones[boneIndex].offsetMatrix;
//         }

//         for (auto const & childIndex : m_nodes[index].child_indices) {
//             nodeIndices.push_back({childIndex, poseMatrix});
//         }
//     }
// }

// int Model::get_texture(aiMaterial &aiMat, aiTextureType type, const char * modelPath) {
//     aiString texPath;
//     int32_t id = -1;
//     if (aiMat.GetTexture(type, 0, &texPath) == AI_SUCCESS) {
//         char fullTexPath[256];
//         strcpy(fullTexPath, modelPath);

//         char *ptr = std::strrchr(fullTexPath, '/');
//         strcpy(++ptr, texPath.C_Str());
//         id = textureManager.loadTexture(fullTexPath, true);
//     }

//     return id;
// }

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
