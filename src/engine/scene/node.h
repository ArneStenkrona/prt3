#ifndef PRT3_NODE_H
#define PRT3_NODE_H

#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/scene/transform.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>

namespace prt3 {

typedef int NodeID;
constexpr NodeID NO_NODE = -1;

class Scene;
class Node {
public:
    Node(Scene & scene);

    Transform get_global_transform() const;
    void set_global_transform(Transform const & transform);
    void set_global_position(glm::vec3 const & position);
    void set_global_rotation(glm::quat const & rotation);
    void set_global_scale(glm::vec3 scale);

    Transform const & local_transform() const { return m_local_transform; }
    Transform & local_transform() { return m_local_transform; }

    NodeID parent_id() { return m_parent_id; }
    std::vector<NodeID> const & children_ids() const { return m_children_ids; }
private:
    Scene & m_scene;

    Transform m_local_transform;
    NodeID m_parent_id = NO_NODE;
    std::vector<NodeID> m_children_ids;

    friend class Scene;
};

} // namespace prt3

#endif
