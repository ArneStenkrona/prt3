#ifndef PRT3_AABB_TREE_H
#define PRT3_AABB_TREE_H

#include "aabb.h"

#include "src/engine/physics/aabb.h"
#include "src/engine/physics/collider.h"

#include <vector>
#include <unordered_map>
#include <cstdint>

namespace prt3 {

typedef int32_t TreeIndex;

// thank you Andy Gaul: https://www.randygaul.net/2013/08/06/dynamic-aabb-tree/
class DynamicAABBTree {
public:
    void clear();

    /**
     * Finds all intersecting nodes for aabb
     * @param aabb aabb of query object
     * @param tags vector to store collider tags of nodes
     */
    void query(ColliderTag caller,
               AABB const & aabb,
               std::vector<ColliderTag> & tags);
    /**
     * Finds all intersecting nodes for aabb
     * @param aabb aabb of query object
     * @param meshIndices vector to store mesh indices
     * @param capsuleIndices vector to store capsule indices
     */
    void query(ColliderTag caller, AABB const & aabb,
               std::array<std::vector<ColliderID>,
               ColliderType::collider_type_none> & ids);

    /**
     * Finds all intersecting nodes for raycast
     * @param origin origin of the ray
     * @param direction direction of the ray
     * @param max_distance maximum length of the ray
     * @param tags vector to store collider tags of nodes
     */
    void query_raycast(glm::vec3 const& origin,
                       glm::vec3 const& direction,
                       float max_distance,
                       std::vector<ColliderTag> & tags);

    /**
     * Inserts aabbs along with their collider tags into the tree
     * @param tag
     * @param aabb
     */
    void insert(ColliderTag const & tag,
                AABB const & aabb);

    /**
     * Inserts aabbs along with their collider tags into the tree
     * @param tags address of the start of the range of collider tags
     * @param aabbs address of the start of the range of aabbs
     * @param n number of aabbs to be inserted
     */
    void insert(ColliderTag const * tags,
                AABB const * aabbs,
                size_t n);

    /**
     * Updates the nodes given by a range of tree indices which is then
     * made up-to-date
     * @param tag address of the start of the range of collider tags
     *             of the aabbs that will be updated
     * @param aabb address of the start of the range of aabbs
     */
    void update(ColliderTag const & tag, AABB const & aabb);

    /**
     * Updates the nodes given by a range of tree indices which is then
     * made up-to-date
     * @param tags address of the start of the range of collider tags
     *             of the aabbs that will be updated
     * @param aabbs address of the start of the range of aabbs
     * @param n number of aabbs to update
     */
    void update(ColliderTag const * tags, AABB const * aabbs, size_t n);

    /**
     * @param tag
     */
    void remove(ColliderTag const & tag);

    /**
     * @param tags address of the start of the range of tags
     *             that will be removed
     * @param n number of nodes to be removed
     */
    void remove(ColliderTag const * tags, size_t n);

    /**
     * The cost heuristic is defined as
     * the total surface area of all nodes
     * excluding the leaf nodes
     * @return the cost of the tree
     */
    float cost() const;

private:
    struct TreeNode;
    static constexpr float buffer = 0.05f;
    TreeIndex m_root_index = TreeNode::null_index;

    TreeIndex m_free_head = TreeNode::null_index; // free list
    TreeIndex m_size = 0;
    std::vector<TreeNode> m_nodes;
    std::unordered_map<ColliderTag, TreeIndex> m_tag_to_index;

    TreeIndex insert_leaf(ColliderTag tag, AABB const & aabb);
    void remove(TreeIndex index);

    void free_tree_node(TreeIndex index);

    TreeIndex allocate_tree_node();

    void synch_hierarchy(TreeIndex index);
    void balance(TreeIndex index);

    TreeIndex find_best_sibling(TreeIndex leafIndex) const;

    struct TreeNodeCost;
    TreeNodeCost sibling_cost(TreeNode const & leaf,
                              TreeIndex siblingIndex,
                              float inherited_cost) const;

    struct TreeNode {
        AABB aabb;

        // leaf = 0, free nodes = -1
        TreeIndex height = 0;

        static constexpr TreeIndex null_index = -1;

        bool is_leaf() const { return right == null_index; }

        union {
            TreeIndex parent = null_index;
            TreeIndex next; // free list
        };

        // union {
            // struct {
                TreeIndex left = null_index;
                TreeIndex right = null_index;
            // };
            // struct {
                ColliderTag collider_tag;
                // int32_t notUsed;
            // };
            // void *userData;
        // };

    };

    struct TreeNodeCost {
        TreeIndex index;
        float direct_cost;
        float inherited_cost;
        friend bool operator<(TreeNodeCost const & lhs,
                              TreeNodeCost const & rhs)
            { return lhs.direct_cost + lhs.inherited_cost <
                     rhs.direct_cost + rhs.inherited_cost; }
    };
};

} // namespace prt3

#endif
