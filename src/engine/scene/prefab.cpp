#include "prefab.h"

#include "src/util/serialization_util.h"
#include "src/util/mem.h"

#include <sstream>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>

using namespace prt3;

Prefab::Prefab(char const * path) {
    std::ifstream file(path, std::ios::binary);

    file.unsetf(std::ios::skipws);
    std::streampos file_size;

    file.seekg(0, std::ios::end);
    file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    m_data.reserve(file_size);

    m_data.insert(
        m_data.begin(),
        std::istream_iterator<char>(file),
        std::istream_iterator<char>()
    );
}

NodeID mapped_parent_id = -2;

void Prefab::serialize_node(
    Scene const & scene,
    NodeID node_id,
    std::ostream & out
) {
    thread_local std::unordered_map<NodeID, NodeID> id_map;
    id_map.clear();
    id_map[NO_NODE] = NO_NODE;

    NodeID serialized_id = 0;

    id_map[scene.get_node(node_id).parent_id()] = mapped_parent_id;

    thread_local std::vector<NodeID> queue;
    queue.push_back(node_id);

    NodeID n_nodes = 0;
    while (!queue.empty()) {
        NodeID id = queue.back();
        queue.pop_back();

        Node const & node = scene.get_node(id);

        id_map[id] = serialized_id;
        ++serialized_id;

        for (NodeID const & child_id : node.children_ids()) {
            queue.push_back(child_id);
        }
        ++n_nodes;
    }

    write_stream(out, n_nodes);

    queue.push_back(node_id);

    while (!queue.empty()) {
        NodeID id = queue.back();
        queue.pop_back();

        Node const & node = scene.get_node(id);

        write_stream(out, id_map.at(id));
        write_stream(out, id_map.at(node.parent_id()));

        auto const & name = scene.get_node_name(node.id());
        out.write(name.data(), name.writeable_size());

        if (scene.node_has_tag(id)) {
            auto const & tag = scene.get_node_tag(id);
            write_stream(out, true);
            out.write(tag.data(), tag.writeable_size());
        } else {
            write_stream(out, false);
        }

        if (id == node_id) {
            Transform tform = node.local_transform();
            tform.position = glm::vec3{0.0f};
            out << tform;
        } else {
            out << node.local_transform();
        }

        // Traverse the children in reverse order in order
        // so that by adding them one bye one results
        // in a vector with the same ordering
        auto it = node.children_ids().end();
        while (it != node.children_ids().begin()) {
            --it;
            queue.push_back(*it);
        }
    }

    queue.push_back(node_id);
    while (!queue.empty()) {
        NodeID id = queue.back();
        queue.pop_back();

        scene.serialize_components(out, id);

        for (NodeID const & child_id : scene.get_node(id).children_ids()) {
            queue.push_back(child_id);
        }
    }
}
NodeID Prefab::instantiate(Scene & scene, NodeID parent) {
    imemstream in(m_data.data(), m_data.size());
    return deserialize_node(scene, parent, in);
}

NodeID Prefab::deserialize_node(Scene & scene, NodeID parent, std::istream & in) {
    thread_local std::unordered_map<NodeID, NodeID> id_map;
    id_map.clear();
    id_map[mapped_parent_id] = parent;

    NodeID n_nodes;
    read_stream(in, n_nodes);

    NodeID ret = NO_NODE;

    for (NodeID i = 0; i < n_nodes; ++i) {
        NodeID s_id;
        read_stream(in, s_id);

        NodeID parent_id;
        read_stream(in, parent_id);

        NodeName name;
        in.read(name.data(), name.writeable_size());
        NodeID id = scene.add_node(id_map.at(parent_id), name);

        bool has_tag;
        read_stream(in, has_tag);
        if (has_tag) {
            NodeTag tag;
            in.read(tag.data(), tag.writeable_size());
            scene.set_node_tag(tag, id);
        }

        id_map[s_id] = id;

        Node & node = scene.get_node(id);
        in >> node.local_transform();

        if (i == 0) {
            ret = id;
        }
    }

    thread_local std::vector<NodeID> queue;
    queue.push_back(ret);
    while (!queue.empty()) {
        NodeID id = queue.back();
        queue.pop_back();

        scene.deserialize_components(in, id);

        for (NodeID const & child_id : scene.get_node(id).children_ids()) {
            queue.push_back(child_id);
        }
    }

    scene.m_script_container.init_unitialized(scene);

    return ret;
}