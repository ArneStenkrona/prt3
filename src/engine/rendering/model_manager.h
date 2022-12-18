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

typedef int32_t ModelHandle;
constexpr ModelHandle NO_MODEL = -1;

class ModelManager {
public:
    ModelManager(Context & context);

    void clear();

    ModelHandle upload_model(std::string const & path);
    ModelHandle upload_animated_model(std::string const & path);

    NodeID add_model_to_scene_from_path(
        std::string const & path,
        Scene & scene,
        NodeID parent_id
    );

    Model const & get_model_from_mesh_id(ResourceID id) const
    { return m_models.at(m_mesh_id_to_model.at(id)); }
    Model const & get_model_from_material_id(ResourceID id) const
    { return m_models.at(m_material_id_to_model.at(id)); }

    ModelHandle get_model_handle_from_mesh_id(ResourceID id) const
    { return m_mesh_id_to_model.at(id); }
    ModelHandle get_model_handle_from_material_id(ResourceID id) const
    { return m_material_id_to_model.at(id); }

    ModelHandle get_model_handle_from_path(std::string const & path) const
    { return m_path_to_model_handle.at(path); }

    int32_t get_mesh_index_from_mesh_id(ResourceID id) const
    { return m_mesh_id_to_mesh_index.at(id); }

    int32_t get_mesh_index_from_material_id(ResourceID id) const
    { return m_material_id_to_mesh_index.at(id); }

    ResourceID get_mesh_id_from_mesh_index(
        ModelHandle handle,
        int32_t mesh_index
    ) const {
        return m_model_resources.at(handle).mesh_resource_ids.at(mesh_index);
    }

    ResourceID get_material_id_from_mesh_index(
        ModelHandle handle,
        int32_t mesh_index
    ) const {
        return m_model_resources.at(handle).material_resource_ids.at(mesh_index);
    }

    Model const & get_model(ModelHandle handle) const
    { return m_models[handle]; }

    std::vector<Model> const & models() const { return m_models; }

    std::unordered_map<ModelHandle, ModelResource> const & model_resources() const
    { return m_model_resources; }

private:
    typedef int ModelResourceIndex;

    Context & m_context;

    std::vector<Model> m_models; // models imported into memory
    std::unordered_map<ModelHandle, ModelResource> m_model_resources; // models uploaded to graphics device
    std::unordered_map<ResourceID, ModelHandle> m_mesh_id_to_model;
    std::unordered_map<ResourceID, ModelHandle> m_material_id_to_model;

    std::unordered_map<ResourceID, int32_t> m_mesh_id_to_mesh_index;
    std::unordered_map<ResourceID, int32_t> m_material_id_to_mesh_index;

    std::unordered_map<std::string, ModelHandle> m_path_to_model_handle;

    NodeID add_model_to_scene(Scene & scene, ModelHandle handle, NodeID parent_id);
    bool model_is_uploaded(ModelHandle handle);
    /* upload to graphics device */
    void upload_model(ModelHandle handle);
};

} // namespace prt3

#endif
