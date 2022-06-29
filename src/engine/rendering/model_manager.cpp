#include "model_manager.h"

#include "src/engine/scene/scene.h"
#include "src/engine/core/context.h"
#include "src/engine/rendering/model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace prt3;

ModelManager::ModelManager(Context & context)
: m_context{context} {

}

void ModelManager::add_model_to_scene_from_path(std::string const & path,
                                                Scene             & scene,
                                                NodeID              parent_id) {
    if (m_path_to_model_handle.find(path) == m_path_to_model_handle.end())
    {
        // load from file
        ModelHandle handle = m_models.size();
        m_models.emplace_back(path.c_str());
        m_path_to_model_handle.insert({path, handle});
    }

    ModelHandle handle = m_path_to_model_handle[path];
    add_model_to_scene(scene, handle, parent_id);
}

void ModelManager::add_model_to_scene(Scene & scene, ModelHandle handle, NodeID parent_id) {
    if (!model_is_uploaded(handle)) {
        upload_model(handle);
    }

    Model const & model = m_models[handle];
    ModelResource const & resource = m_model_resources[handle];

    struct QueueElement {
        unsigned int model_node_index;
        NodeID parent_id;
    };

    std::vector<QueueElement> queue{ { 0, parent_id} };

    while (!queue.empty()) {
        size_t model_node_index = queue.back().model_node_index;
        NodeID parent_id = queue.back().parent_id;
        queue.pop_back();

        NodeID node_id = scene.add_node(parent_id);

        Model::Node const & model_node = model.nodes()[model_node_index];
        if (model_node.mesh_index != -1) {
            ResourceID mesh_id = resource.mesh_resource_ids[model_node.mesh_index];
            scene.set_node_mesh(node_id, mesh_id);
        }

        for (uint32_t const & index : model_node.child_indices) {
            queue.push_back({static_cast<unsigned int>(index), node_id});
        }
    }
}

bool ModelManager::model_is_uploaded(ModelHandle handle) {
    return m_model_resources.find(handle) != m_model_resources.end();
}

void ModelManager::upload_model(ModelHandle handle) {
    Model const & model = m_models[handle];
    ModelResource & resource = m_model_resources[handle];

    m_context.renderer().upload_model(model, resource.mesh_resource_ids);
}

