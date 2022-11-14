#include "transform_cache.h"

using namespace prt3;

void TransformCache::collect_global_transforms(Node const * nodes,
                                               size_t n_nodes,
                                               NodeID root_id) {
    if (root_id == NO_NODE) {
        return;
    }
    m_global_transforms.resize(n_nodes);
    m_global_transforms_history = m_global_transforms;

    static std::vector<NodeID> queue;
    queue.push_back(root_id);

    m_global_transforms[root_id] = nodes[root_id].local_transform();

    while (!queue.empty()) {
        NodeID node_id = queue.back();
        Node const & node = nodes[node_id];

        Transform node_tform = m_global_transforms[node_id];
        queue.pop_back();

        for (NodeID const & child_id : node.children_ids()) {
            Node const & child = nodes[child_id];
            Transform child_tform = child.local_transform();

            child_tform.position = node_tform.position +
                glm::rotate(
                    node_tform.rotation,
                    node_tform.scale * child_tform.position
                );
            child_tform.scale = child_tform.scale * node_tform.scale;
            child_tform.rotation = node_tform.rotation * child_tform.rotation;

            m_global_transforms[child_id] = child_tform;
            queue.push_back(child_id);
        }
    }
}

void TransformCache::clear() {
    m_global_transforms.clear();
    m_global_transforms_history.clear();
}
