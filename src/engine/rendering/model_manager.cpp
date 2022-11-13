#include "model_manager.h"

#include "src/engine/scene/scene.h"
#include "src/engine/core/context.h"
#include "src/engine/rendering/model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>

using namespace prt3;

ModelManager::ModelManager(Context & context)
: m_context{context} {

}

void ModelManager::clear() {
    for (auto const & pair : m_model_resources) {
        m_context.renderer().free_model(pair.first, pair.second);
    }

    m_models.clear();
    m_model_resources.clear();
    m_mesh_id_to_model.clear();
    m_material_id_to_model.clear();
    m_mesh_id_to_mesh_index.clear();
    m_material_id_to_mesh_index.clear();
    m_path_to_model_handle.clear();
}

ModelManager::ModelHandle ModelManager::upload_model(
    std::string const & path
) {
    if (m_path_to_model_handle.find(path) == m_path_to_model_handle.end()) {
        // load from file
        ModelHandle handle = m_models.size();
        m_models.emplace_back(path.c_str());
        m_path_to_model_handle.insert({path, handle});
        upload_model(handle);
    }

    ModelHandle handle = m_path_to_model_handle[path];

    return handle;
}

NodeID ModelManager::add_model_to_scene_from_path(
    std::string const & path,
    Scene             & scene,
    NodeID              parent_id
) {
    if (m_path_to_model_handle.find(path) == m_path_to_model_handle.end()) {
        // load from file
        ModelHandle handle = m_models.size();
        m_models.emplace_back(path.c_str());
        m_path_to_model_handle.insert({path, handle});
    }

    ModelHandle handle = m_path_to_model_handle[path];
    return add_model_to_scene(scene, handle, parent_id);
}

NodeID ModelManager::add_model_to_scene(
    Scene & scene,
    ModelHandle handle,
    NodeID parent_id
) {
    if (!model_is_uploaded(handle)) {
        upload_model(handle);
    }

    Model const & model = m_models[handle];
    ModelResource const & resource = m_model_resources[handle];

    struct QueueElement {
        uint32_t model_node_index;
        NodeID parent_id;
    };

    NodeID model_node_id = scene.get_next_available_node_id();

    std::vector<QueueElement> queue{ { 0, parent_id} };

    while (!queue.empty()) {
        size_t model_node_index = queue.back().model_node_index;
        NodeID parent_id = queue.back().parent_id;
        queue.pop_back();

        Model::Node const & model_node = model.nodes()[model_node_index];
        NodeID node_id = scene.add_node(parent_id, model_node.name.c_str());

        if (model_node.mesh_index != -1) {
            ResourceID mesh_id = resource.mesh_resource_ids[model_node.mesh_index];
            ResourceID material_id = resource.material_resource_ids[model_node.mesh_index];

            scene.add_component<Mesh>(node_id, mesh_id);
            scene.add_component<MaterialComponent>(node_id, material_id);
        }

        for (uint32_t const & index : model_node.child_indices) {
            queue.push_back({index, node_id});
        }
    }

    std::string name{model.name().c_str()};
    size_t dot_pos = name.find('.');
    if (dot_pos != std::string::npos) {
        name = name.substr(0, dot_pos);
    }
    scene.get_node_name(model_node_id) = name.c_str();
    return model_node_id;
}

bool ModelManager::model_is_uploaded(ModelHandle handle) {
    return m_model_resources.find(handle) != m_model_resources.end();
}

void ModelManager::upload_model(ModelHandle handle) {
    Model const & model = m_models[handle];
    ModelResource & resource = m_model_resources[handle];

    m_context.renderer().upload_model(handle, model, resource);

    std::vector<uint32_t> queue{ 0 };
    while (!queue.empty()) {
        size_t model_node_index = queue.back();
        queue.pop_back();

        Model::Node const & model_node = model.nodes()[model_node_index];
        auto & manager_material_ids = m_context.material_manager().m_material_ids;
        if (model_node.mesh_index != -1) {
            ResourceID mesh_id = resource.mesh_resource_ids[model_node.mesh_index];
            ResourceID material_id = resource.material_resource_ids[model_node.mesh_index];

            m_mesh_id_to_model[mesh_id] = handle;
            m_material_id_to_model[material_id] = handle;

            m_mesh_id_to_mesh_index[mesh_id] = model_node.mesh_index;
            m_material_id_to_mesh_index[material_id] = model_node.mesh_index;

            manager_material_ids.insert(material_id);
        }

        for (uint32_t const & index : model_node.child_indices) {
            queue.push_back(index);
        }
    }
}
