#include "aabb_tree.h"

#include <algorithm>
#include <queue>
#include <cassert>

using namespace prt3;

void DynamicAABBTree::clear() {
    m_root_index = TreeNode::null_index;

    m_free_head = TreeNode::null_index;
    m_size = 0;
    m_nodes.clear();
    m_tag_to_index.clear();
}

void DynamicAABBTree::query(ColliderTag caller,
                            AABB const & aabb,
                            std::vector<ColliderTag> & tags) {
    if (m_size == 0) {
        return;
    }

    thread_local std::vector<TreeIndex> node_stack;
    node_stack.push_back(m_root_index);
    while (!node_stack.empty()) {
        TreeIndex index = node_stack.back();
        node_stack.pop_back();
        TreeNode const & node = m_nodes[index];
        if (AABB::intersect(node.aabb, aabb)) {
            if (node.is_leaf()) {
                if (caller != node.collider_tag) {
                    tags.push_back(node.collider_tag);
                }
            } else {
                node_stack.push_back(node.left);
                node_stack.push_back(node.right);
            }
        }
    }
}

void DynamicAABBTree::query(
    ColliderTag caller,
    AABB const & aabb,
    std::array<std::vector<ColliderID>,
        ColliderType::total_num_collider_type> & ids) {
    if (m_size == 0) {
        return;
    }

    thread_local std::vector<TreeIndex> node_stack;
    node_stack.push_back(m_root_index);
    while (!node_stack.empty()) {
        TreeIndex index = node_stack.back();
        node_stack.pop_back();
        TreeNode const & node = m_nodes[index];
        if (AABB::intersect(node.aabb, aabb)) {
            if (node.is_leaf()) {
                if (caller != node.collider_tag) {
                    ids[node.collider_tag.type].push_back(node.collider_tag.id);
                }
            } else {
                node_stack.push_back(node.left);
                node_stack.push_back(node.right);
            }
        }
    }
}

void DynamicAABBTree::query_raycast(glm::vec3 const& origin,
                                    glm::vec3 const& direction,
                                    float max_distance,
                                    std::vector<ColliderTag> & tags) {
    if (m_size == 0) {
        return;
    }

    thread_local std::vector<TreeIndex> node_stack;
    node_stack.push_back(m_root_index);
    while (!node_stack.empty()) {
        TreeIndex index = node_stack.back();
        node_stack.pop_back();
        TreeNode const & node = m_nodes[index];
        if (AABB::intersect_ray(node.aabb, origin, direction, max_distance)) {
            if (node.is_leaf()) {
                tags.push_back(node.collider_tag);
            } else {
                node_stack.push_back(node.left);
                node_stack.push_back(node.right);
            }
        }
    }
}

void DynamicAABBTree::insert(ColliderTag const & tag,
                             AABB const & aabb) {
    insert_leaf(tag, aabb);
}

void DynamicAABBTree::insert(ColliderTag const * tags,
                             AABB const * aabbs,
                             size_t n) {
    for (size_t i = 0; i < n; ++i) {
        insert_leaf(tags[i], aabbs[i]);
    }
}

void DynamicAABBTree::update(ColliderTag const & tag, AABB const & aabb) {
    TreeIndex index = m_tag_to_index[tag];
    TreeNode & node = m_nodes[index];
    if (!node.aabb.contains(aabb)) {
        ColliderTag tag = node.collider_tag;
        remove(m_tag_to_index[tag]);
        insert_leaf(tag, aabb);
    }
}

void DynamicAABBTree::update(ColliderTag const * tags,
                             AABB const * aabbs,
                             size_t n) {
    for (size_t i = 0; i < n; ++i) {
        ColliderTag const & tag = tags[i];
        TreeIndex index = m_tag_to_index[tag];
        TreeNode & node = m_nodes[index];
        if (!node.aabb.contains(aabbs[i])) {
            ColliderTag tag = node.collider_tag;
            remove(m_tag_to_index[tag]);
            insert_leaf(tag, aabbs[i]);
        }
    }
}

void DynamicAABBTree::remove(ColliderTag const & tag) {
    remove(m_tag_to_index[tag]);
}

void DynamicAABBTree::remove(ColliderTag const * tags, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        remove(m_tag_to_index[tags[i]]);
    }
}

float DynamicAABBTree::cost() const {
    float cost = 0.0f;
    for (auto const & node : m_nodes) {
        cost += node.aabb.area();
    }
    return cost;
}

TreeIndex DynamicAABBTree::insert_leaf(ColliderTag tag,
                                       AABB const & aabb) {
    // insert new node into vector
    TreeIndex leaf_index = allocate_tree_node();

    m_nodes[leaf_index].collider_tag = tag;
    // add buffer to aabb
    m_nodes[leaf_index].aabb.lower_bound = aabb.lower_bound - buffer;
    m_nodes[leaf_index].aabb.upper_bound = aabb.upper_bound + buffer;

    if (m_root_index == TreeNode::null_index) {
        m_root_index = leaf_index;
        return leaf_index;
    }
    // traverse tree to find suitable place of insertion
    // stage 1: find the best sibling for the new leaf
    TreeIndex sibling_index = find_best_sibling(leaf_index);

    // stage 2: create a new parent
    TreeIndex old_parent_index = m_nodes[sibling_index].parent;
    // warning, this may invalidate references
    TreeIndex new_parent_index = allocate_tree_node();
    TreeNode & new_parent = m_nodes[new_parent_index];
    new_parent.parent = old_parent_index;
    new_parent.aabb = m_nodes[leaf_index].aabb + m_nodes[sibling_index].aabb;
    new_parent.height = 1 + m_nodes[sibling_index].height;

    if (old_parent_index != TreeNode::null_index) {
        TreeNode & old_parent = m_nodes[old_parent_index];
        // The sibling was not the root
        if (old_parent.left == sibling_index) {
            old_parent.left = new_parent_index;
        } else {
            old_parent.right = new_parent_index;
        }
    } else {
        // The sibling was the root
        m_root_index = new_parent_index;
    }
    new_parent.left = sibling_index;
    new_parent.right = leaf_index;
    m_nodes[sibling_index].parent = new_parent_index;
    m_nodes[leaf_index].parent = new_parent_index;

    // stage 3: walk back up the tree refitting AABBs and applying rotations
    synch_hierarchy(m_nodes[leaf_index].parent);

    m_tag_to_index[tag] = leaf_index;
    return leaf_index;
}

void DynamicAABBTree::remove(TreeIndex index) {
    TreeNode & n = m_nodes[index];
    assert(n.is_leaf());

    if (index == m_root_index) {
        m_root_index = TreeNode::null_index;
        free_tree_node(index);
        return;
    }

    TreeIndex parent = n.parent;

    TreeIndex sibling_index;
    if (m_nodes[parent].left == index) {
        sibling_index = m_nodes[parent].right;
    } else {
        sibling_index = m_nodes[parent].left;
    }

    if (m_nodes[parent].parent != TreeNode::null_index)  {
        TreeIndex grandparent = m_nodes[parent].parent;
        if (m_nodes[grandparent].left == n.parent) {
            m_nodes[grandparent].left = sibling_index;
        } else {
            m_nodes[grandparent].right = sibling_index;
        }

        m_nodes[sibling_index].parent = grandparent;
        free_tree_node(n.parent);

        synch_hierarchy(grandparent);
    } else {
        m_root_index = sibling_index;
        m_nodes[sibling_index].parent = TreeNode::null_index;
        free_tree_node(n.parent);
    }

    free_tree_node(index);
}

void DynamicAABBTree::free_tree_node(TreeIndex index) {
    m_nodes[index].next = m_free_head;
    m_nodes[index].height = -1;
    m_free_head = index;
    --m_size;
}

TreeIndex DynamicAABBTree::allocate_tree_node() {
    TreeIndex index;
    if (m_free_head == TreeNode::null_index) {
        index = m_nodes.size();
        m_nodes.push_back({});
    } else {
        index = m_free_head;
        m_free_head = m_nodes[m_free_head].next;
        m_nodes[index] = {};
    }
    ++m_size;
    return index;
}

void DynamicAABBTree::synch_hierarchy(TreeIndex index) {
    while (index != TreeNode::null_index) {
        balance(index);

        TreeIndex left = m_nodes[index].left;
        TreeIndex right = m_nodes[index].right;

        m_nodes[index].height = 1
            + std::max(m_nodes[left].height, m_nodes[right].height);
        m_nodes[index].aabb = m_nodes[left].aabb + m_nodes[right].aabb;

        // balance(index);
        index = m_nodes[index].parent;
    }
}

void DynamicAABBTree::balance(TreeIndex index) {
    TreeNode & node = m_nodes[index];
    if (node.height < 2) {
        return;
    }

    TreeIndex ileft = node.left;
    TreeIndex iright = node.right;
    TreeNode & left = m_nodes[ileft];
    TreeNode & right = m_nodes[iright];

    TreeIndex difference = left.height - right.height;
    if (difference > 1) {
        // left is higher than right, rotate left up
        TreeIndex * ia = m_nodes[left.left].height > m_nodes[left.right].height ?
                       &left.left : &left.right;
        TreeNode & a = m_nodes[*ia];
        right.parent = ileft;
        a.parent = index;

        node.right = *ia;
        *ia = iright;

        left.aabb = m_nodes[left.left].aabb + m_nodes[left.right].aabb;
        left.height = std::max(m_nodes[left.left].height,
                               m_nodes[left.right].height) + 1;
    } else if (difference < -1) {
        // right is higher than left, rotate right up
        TreeIndex * ia = m_nodes[right.left].height >
                       m_nodes[right.right].height ?
                       &right.left : &right.right;
        TreeNode & a = m_nodes[*ia];
        left.parent = iright;
        a.parent = index;

        node.left = *ia;
        *ia = ileft;

        right.aabb = m_nodes[right.left].aabb + m_nodes[right.right].aabb;
        right.height = std::max(m_nodes[right.left].height,
                                m_nodes[right.right].height) + 1;
    }
    node.height = std::max(m_nodes[node.left].height,
                           m_nodes[node.right].height) + 1;
}

TreeIndex DynamicAABBTree::find_best_sibling(TreeIndex leaf_index) const {
    TreeNode const & leaf = m_nodes[leaf_index];
    TreeIndex best_sibling = m_root_index;
    float best_cost = (leaf.aabb + m_nodes[m_root_index].aabb).area();
    thread_local std::priority_queue<TreeNodeCost> q;

    q.push(sibling_cost(leaf, m_root_index, 0.0f));
    while (!q.empty()) {
        TreeNodeCost nc = q.top();
        q.pop();
        float currentCost = nc.direct_cost + nc.inherited_cost;
        if (currentCost < best_cost) {
            best_cost = currentCost;
            best_sibling = nc.index;
        }

        TreeNode const & curr = m_nodes[nc.index];
        if (!curr.is_leaf()) {
            float inherited_cost = nc.inherited_cost
                + (leaf.aabb + curr.aabb).area() - curr.aabb.area();
            float cLow = leaf.aabb.area() + inherited_cost;
            if (cLow < best_cost) {
                TreeNodeCost left = sibling_cost(leaf, curr.left,
                                                 inherited_cost);
                TreeNodeCost right = sibling_cost(leaf, curr.right,
                                                  inherited_cost);
                q.push(left);
                q.push(right);
            }
        }
    }

    return best_sibling;
}

DynamicAABBTree::TreeNodeCost DynamicAABBTree::sibling_cost(
    TreeNode const & leaf,
    TreeIndex sibling_index,
    float inherited_cost
) const {
    TreeNodeCost cost;
    cost.index = sibling_index;

    float direct_cost = (leaf.aabb + m_nodes[sibling_index].aabb).area();

    cost.direct_cost = direct_cost;
    cost.inherited_cost = inherited_cost;
    return cost;
}
