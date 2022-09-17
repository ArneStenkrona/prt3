#ifndef PRT3_MODEL_MANAGER_H
#define PRT3_MODEL_MANAGER_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/model.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <assimp/scene.h>

#include <vector>
#include <string>
#include <unordered_map>


namespace prt3 {

class Scene;
class Context;

struct ModelResource {
    std::vector<ResourceID> mesh_resource_ids;
    std::vector<ResourceID> material_resource_ids;
};

class ModelManager {
public:

    typedef int ModelHandle;

    ModelManager(Context & context);

    NodeID add_model_to_scene_from_path(std::string const & path, Scene & scene, NodeID parent_id);

private:
    typedef int ModelResourceIndex;

    Context & m_context;

    std::vector<Model> m_models; // models imported into memory
    std::unordered_map<ModelHandle, ModelResource> m_model_resources; // models uploaded to graphics device
    std::unordered_map<ResourceID, ModelHandle> m_mesh_id_to_model;
    std::unordered_map<ResourceID, ModelHandle> m_material_id_to_model;

    // std::unordered_map<ModelHandle, ModelResourceIndex> m_handle_to_resource_index;
    std::unordered_map<std::string, ModelHandle> m_path_to_model_handle;

    NodeID add_model_to_scene(Scene & scene, ModelHandle handle, NodeID parent_id);
    bool model_is_uploaded(ModelHandle handle);
    /* upload to graphics device */
    void upload_model(ModelHandle handle);

    friend class MeshManager;
};

} // namespace prt3

#endif
