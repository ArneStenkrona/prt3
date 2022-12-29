#include "armature.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

Armature::Armature(Scene &, NodeID node_id)
 : m_node_id{node_id} {
}

Armature::Armature(
    Scene & scene,
    NodeID node_id,
    ModelHandle model_handle
)
 : m_node_id{node_id},
   m_model_handle{model_handle}
{
    if (m_model_handle != NO_MODEL) {
        m_animation_id =
            scene.animation_system().add_animation(scene, model_handle);
    }
}

Armature::Armature(
    Scene & scene,
    NodeID node_id,
    std::istream & in
)
 : m_node_id{node_id}
{
    ModelManager & man = scene.model_manager();

    size_t n_path;
    read_stream(in, n_path);

    if (n_path > 0) {
        static std::string path;
        path.resize(n_path);

        in.read(path.data(), path.size());

        m_model_handle = man.upload_model(path);
    }

    if (m_model_handle != NO_MODEL) {
        m_animation_id =
            scene.animation_system().add_animation(scene, m_model_handle);
    }
}

void Armature::set_model_handle(
    Scene & scene,
    ModelHandle handle
) {
    m_model_handle = handle;
    if (handle != NO_MODEL) {
        if (m_animation_id == NO_ANIMATION) {
            m_animation_id =
                scene.animation_system().add_animation(scene, handle);
        } else {
            scene.animation_system().init_animation(scene, handle, m_animation_id);
        }
    } else {
        scene.animation_system().remove_animation(m_animation_id);
    }
}

void Armature::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    if (m_model_handle != NO_MODEL) {
        ModelManager const & man = scene.model_manager();

        Model const & model = man.get_model(m_model_handle);

        std::string const & path = model.path();

        write_stream(out, path.size());
        out.write(path.data(), path.size());
    } else {
        write_stream(out, 0);
    }
}

bool Armature::validate_node_children(Scene & scene) {
    // TODO: A solution that doesn't require expensive traversal.
    //       Perhaps a flag that indicates if child nodes have
    //       changed.
    ModelManager const & man = scene.model_manager();

    Model const & model = man.get_model(m_model_handle);

    thread_local std::vector<uint32_t> indices;
    indices.push_back(0);

    thread_local std::unordered_set<NodeName> bone_names;
    bone_names.clear();

    for (Model::Node const & node : model.nodes()) {
        if (node.bone_index != -1) {
            bone_names.insert(node.name.c_str());
        }
    }

    Node const & armature_node = scene.get_node(m_node_id);

    thread_local std::vector<NodeID> ids;
    for (NodeID id : armature_node.children_ids()) {
        ids.push_back(id);
    }

    while (!ids.empty()) {
        NodeID id = ids.back();
        ids.pop_back();
        Node const & node = scene.get_node(id);

        NodeName const & name = scene.get_node_name(id);

        if (bone_names.find(name) != bone_names.end()) {
            bone_names.erase(name);
        }

        for (NodeID child_id : node.children_ids()) {
            ids.push_back(child_id);
        }
    }

    return bone_names.empty();
}

void Armature::recreate_nodes(Scene & scene) {
    // TODO: A solution that doesn't require expensive traversal.
    //       Perhaps a flag that indicates if child nodes have
    //       changed.
    ModelManager const & man = scene.model_manager();

    Model const & model = man.get_model(m_model_handle);

    struct QueueElement {
        uint32_t index;
        NodeID id;
        NodeID parent_id;
    };

    thread_local std::vector<QueueElement> queue;
    queue.push_back({0, m_node_id, NO_NODE});

    while (!queue.empty()) {
        QueueElement elem = queue.back();
        queue.pop_back();

        Model::Node const & mn = model.nodes()[elem.index];

        NodeID node_id = elem.id;
        if (elem.id == NO_NODE) {
            node_id = scene.add_node(elem.parent_id, mn.name.c_str());
        }

        Node const & node = scene.get_node(node_id);

        thread_local std::unordered_map<NodeName, NodeID> existing_nodes;
        existing_nodes.clear();

        for (NodeID child_id : node.children_ids()) {
            existing_nodes[scene.get_node_name(child_id)] = child_id;
        }

        for (uint32_t child_index : mn.child_indices) {
            Model::Node const & cmn = model.nodes()[child_index];
            if (existing_nodes.find(cmn.name.c_str()) != existing_nodes.end()) {
                queue.push_back(
                    {child_index, existing_nodes.at(cmn.name.c_str()), node_id}
                );
            } else {
                queue.push_back({child_index, NO_NODE, node_id});
            }
        }
    }

    m_mapped = false;
}

void Armature::map_bones(Scene & scene) {
    m_bone_map.clear();
    ModelManager const & man = scene.model_manager();

    Model const & model = man.get_model(m_model_handle);

    thread_local std::vector<uint32_t> indices;
    indices.push_back(0);

    thread_local std::unordered_map<NodeName, uint32_t> name_to_index;
    name_to_index.clear();

    while (!indices.empty()) {
        uint32_t index = indices.back();
        indices.pop_back();
        Model::Node const & node = model.nodes()[index];

        if (node.bone_index != -1) {
            name_to_index[node.name.c_str()] = node.bone_index;
        }

        for (uint32_t child_index : node.child_indices) {
            indices.push_back(child_index);
        }
    }

    Node const & armature_node = scene.get_node(m_node_id);

    thread_local std::vector<NodeID> ids;
    for (NodeID id : armature_node.children_ids()) {
        ids.push_back(id);
    }

    while (!ids.empty()) {
        NodeID id = ids.back();
        ids.pop_back();
        Node const & node = scene.get_node(id);

        NodeName const & name = scene.get_node_name(id);

        if (name_to_index.find(name) != name_to_index.end()) {
            uint32_t index = name_to_index.at(name);
            m_bone_map.push_back({id, index});
        }

        for (NodeID child_id : node.children_ids()) {
            ids.push_back(child_id);
        }
    }

    m_mapped = true;
}

void Armature::update_bone_transforms(Scene & scene) {
    Animation const & animation =
        scene.animation_system().get_animation(m_animation_id);

    for (BonePair const & pair : m_bone_map) {
        Node & node = scene.get_node(pair.node_id);
        node.local_transform() = animation.local_transforms[pair.bone_index];
    }
}

void Armature::remove(Scene & scene) {
    if (m_animation_id != NO_ANIMATION) {
        scene.animation_system().remove_animation(m_animation_id);
    }
}

void Armature::update(Scene & scene) {
    if (!validate_node_children(scene)) {
        recreate_nodes(scene);
    }

    if (!m_mapped) {
        map_bones(scene);
    }

    update_bone_transforms(scene);
}
