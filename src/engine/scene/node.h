#ifndef PRT3_NODE_H
#define PRT3_NODE_H

#include "src/engine/rendering/resources.h"
#include "src/engine/component/transform.h"
#include "src/engine/physics/gjk.h"
#include "src/engine/physics/collider.h"
#include "src/util/fixed_string.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>

namespace prt3 {

typedef int32_t NodeID;
constexpr NodeID NO_NODE = -1;

typedef FixedString<64> NodeName;
typedef FixedString<32> NodeTag;
typedef uint8_t ModFlagIntType;

class Scene;
class Node {
public:
    enum ModFlags : ModFlagIntType {
        none = 0,
        descendant_removed = 1 << 0,
        descendant_added = 1 << 1
    };

    Node(NodeID id);

    Transform get_global_transform(Scene const & scene) const;
    void set_global_transform(Scene const & scene, Transform const & transform);
    void set_global_position(Scene const & scene, glm::vec3 const & position);
    void set_global_rotation(Scene const & scene, glm::quat const & rotation);
    void set_global_scale(Scene const & scene, glm::vec3 scale);

    Transform global_to_local_transform(
        Scene const & scene,
        Transform const & transform
    ) const;

    Transform const & local_transform() const { return m_local_transform; }
    Transform & local_transform() { return m_local_transform; }

    NodeID id() const { return m_id; }
    NodeID parent_id() const { return m_parent_id; }
    std::vector<NodeID> const & children_ids() const { return m_children_ids; }

    CollisionResult move_and_collide(
        Scene & scene,
        glm::vec3 const & movement
    );

private:
    Transform m_local_transform;
    NodeID m_id;
    NodeID m_parent_id = NO_NODE;
    std::vector<NodeID> m_children_ids;

    friend class Scene;
    friend class Prefab;
};

} // namespace prt3

inline prt3::Node::ModFlags operator|(prt3::Node::ModFlags a, prt3::Node::ModFlags b)
{
    return static_cast<prt3::Node::ModFlags>(
        static_cast<prt3::ModFlagIntType>(a) |
        static_cast<prt3::ModFlagIntType>(b)
    );
}

inline prt3::Node::ModFlags operator&(prt3::Node::ModFlags a, prt3::Node::ModFlags b)
{
    return static_cast<prt3::Node::ModFlags>(
        static_cast<prt3::ModFlagIntType>(a) &
        static_cast<prt3::ModFlagIntType>(b)
    );
}

#endif
