#include "action_remove_node.h"

#include "src/engine/editor/editor_context.h"
#include "src/util/serialization_util.h"
#include "src/util/mem.h"

#include <sstream>
#include <iostream>

using namespace prt3;

ActionRemoveNode::ActionRemoveNode(
    EditorContext & editor_context,
    NodeID node_id
) : m_editor_context{&editor_context},
    m_node_id{node_id} {

    std::stringstream stream;

    serialize_node(stream);

    std::string const & s = stream.str();
    m_data.reserve(s.size());
    m_data.assign(s.begin(), s.end());
}

bool ActionRemoveNode::apply() {
    Scene & scene = m_editor_context->scene();

    NodeID parent_id = scene.get_node(m_node_id).parent_id();
    scene.remove_node(m_node_id);

    m_editor_context->set_selected_node(parent_id);
    return true;
}

bool ActionRemoveNode::unapply() {
    imemstream in(m_data.data(), m_data.size());

    deserialize_node(in);
    m_editor_context->set_selected_node(m_node_id);

    return true;
}

void ActionRemoveNode::serialize_node(std::ostream & out) const {
    Scene const & scene = m_editor_context->scene();

    thread_local std::vector<NodeID> queue;
    queue.push_back(m_node_id);

    NodeID n_nodes = 0;
    while (!queue.empty()) {
        NodeID id = queue.back();
        queue.pop_back();

        Node const & node = scene.get_node(id);

        for (NodeID const & child_id : node.children_ids()) {
            queue.push_back(child_id);
        }
        ++n_nodes;
    }

    write_stream(out, n_nodes);

    queue.push_back(m_node_id);

    while (!queue.empty()) {
        NodeID id = queue.back();
        queue.pop_back();

        Node const & node = scene.get_node(id);

        auto const & name = scene.get_node_name(node.id());
        out.write(name.data(), name.writeable_size());
        write_stream(out, node.parent_id());
        out << node.local_transform();

        scene.serialize_components(out, id);

        write_stream(out, id);

        // Traverse the children in reverse order in order
        // so that by adding them one bye one results
        // in a vector with the same ordering
        auto it = node.children_ids().end();
        while (it != node.children_ids().begin()) {
            --it;
            queue.push_back(*it);
        }
    }
}

void ActionRemoveNode::deserialize_node(std::istream & in) {
    Scene & scene = m_editor_context->scene();

    NodeID n_nodes;
    read_stream(in, n_nodes);

    for (NodeID i = 0; i < n_nodes; ++i) {
        NodeName name;
        in.read(name.data(), name.writeable_size());

        NodeID parent_id;
        read_stream(in, parent_id);

        NodeID id = scene.add_node(parent_id, name);

        Node & node = scene.get_node(id);
        in >> node.local_transform();

        scene.deserialize_components(in, id);

        NodeID s_id;
        read_stream(in, s_id);
        if (s_id != id) {
            std::cerr << "Node incorrectly serialized/deserialized: "
                      << "expected id = "
                      << id
                      << " but got id = "
                      << s_id
                      << std::endl;
        }
    }
}
