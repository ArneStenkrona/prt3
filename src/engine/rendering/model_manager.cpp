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
    m_free_handles.clear();
}

void ModelManager::free_model(ModelHandle handle) {
    m_context.renderer().free_model(handle, m_model_resources.at(handle));

    {
        ModelResource const & res = m_model_resources.at(handle);
        for (ResourceID id : res.mesh_resource_ids) {
            m_mesh_id_to_model.erase(id);
            m_mesh_id_to_mesh_index.erase(id);
        }
        for (ResourceID id : res.material_resource_ids) {
            m_material_id_to_model.erase(id);
            m_material_id_to_mesh_index.erase(id);
        }
        m_model_resources.erase(handle);
    }

    m_path_to_model_handle.erase(m_models[handle].path());
    m_model_resources.erase(handle);
    m_models[handle] = Model{};

    m_free_handles.push_back(handle);
}

ModelHandle ModelManager::upload_model(
    std::string const & path
) {
    if (m_path_to_model_handle.find(path) == m_path_to_model_handle.end()) {
        // load from file
        ModelHandle handle;
        if (m_free_handles.empty()) {
            handle = m_models.size();
            m_models.emplace_back(path.c_str());
        } else {
            handle = m_free_handles.back();
            m_free_handles.pop_back();
            new(&m_models[handle]) Model{path.c_str()};
        }

        if (!m_models.back().valid()) {
            m_models.pop_back();
            return NO_MODEL;
        }

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
        ModelHandle handle;
        if (m_free_handles.empty()) {
            handle = m_models.size();
            m_models.emplace_back(path.c_str());
        } else {
            handle = m_free_handles.back();
            m_free_handles.pop_back();
            new(&m_models[handle]) Model{path.c_str()};
        }
    }

    ModelHandle handle = m_path_to_model_handle[path];
    return add_model_to_scene(scene, handle, parent_id);
}

NodeID ModelManager::add_model_to_scene(
    Scene & scene,
    ModelHandle handle,
    NodeID base_node,
    bool use_base_as_model_root,
    bool as_animated,
    std::vector<NodeID> & new_nodes
) {
    new_nodes.clear();

    if (!model_is_uploaded(handle)) {
        upload_model(handle);
    }

    Model const & model = m_models[handle];
    ModelResource const & resource = m_model_resources[handle];

    struct QueueElement {
        uint32_t model_node_index;
        NodeID parent_id;
    };

    NodeID model_node_id = use_base_as_model_root ?
                           base_node :
                           scene.get_next_available_node_id();
    thread_local std::vector<QueueElement> queue;

    if (use_base_as_model_root) {
        Model::Node const & model_node = model.nodes()[0];

        if (model_node.mesh_index != -1) {
            assert(false);
        }

        for (uint32_t const & index : model_node.child_indices) {
            queue.push_back({index, base_node});
        }
    } else {
        queue.push_back({ 0, base_node});

    }

    if (as_animated) {
        scene.add_component<Armature>(model_node_id, handle);
    }

    while (!queue.empty()) {
        size_t model_node_index = queue.back().model_node_index;
        NodeID parent_id = queue.back().parent_id;
        queue.pop_back();

        Model::Node const & model_node = model.nodes()[model_node_index];
        NodeID node_id = scene.add_node(parent_id, model_node.name.c_str());
        new_nodes.push_back(node_id);
        scene.get_node(node_id).local_transform() = model_node.transform;

        if (model_node.mesh_index != -1) {
            ResourceID mesh_id = resource.mesh_resource_ids[model_node.mesh_index];
            ResourceID material_id = resource.material_resource_ids[model_node.mesh_index];

            if (as_animated) {
                scene.add_component<AnimatedMesh>(node_id, mesh_id, model_node_id);
            } else {
                scene.add_component<Mesh>(node_id, mesh_id);
            }
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

    if (!use_base_as_model_root) {
        scene.get_node_name(model_node_id) = name.c_str();
    }

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
