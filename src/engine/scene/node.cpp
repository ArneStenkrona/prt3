#include "node.h"

#include "src/engine/scene/scene.h"

#include <glm/gtx/matrix_decompose.hpp>

using namespace prt3;

Node::Node(Scene & scene)
 : m_scene{scene} {

}

Transform Node::get_global_transform() const {
    NodeID curr_id = m_parent_id;
    glm::mat4 curr_tform = m_local_transform.to_matrix();

    while (curr_id != NO_NODE) {
        Node const & curr = m_scene.get_node(curr_id);
        curr_tform = curr_tform * curr.local_transform().to_matrix();
        curr_id = curr.m_parent_id;
    }

    Transform transform;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(curr_tform,
                   transform.scale,
                   transform.rotation,
                   transform.position,
                   skew,
                   perspective);

    return transform;
}
void Node::set_global_transform(Transform const & transform) {
    Transform global = get_global_transform();
    glm::vec3 delta_pos = transform.position - global.position;
    glm::vec3 delta_scale = transform.scale / global.scale;
    glm::quat delta_rot = transform.rotation
        * glm::inverse(global.rotation);

    m_local_transform.position = m_local_transform.position + delta_pos;
    m_local_transform.scale = m_local_transform.scale * delta_scale;
    m_local_transform.rotation = delta_rot * m_local_transform.rotation;
}

void Node::set_global_position(glm::vec3 const & position) {
    Transform global = get_global_transform();
    glm::vec3 delta_pos = position - global.position;

    m_local_transform.position = m_local_transform.position + delta_pos;
}

void Node::set_global_rotation(glm::quat const & rotation) {
    Transform global = get_global_transform();
    glm::quat delta_rot = rotation
        * glm::inverse(global.rotation);

    m_local_transform.rotation = delta_rot * m_local_transform.rotation;
}
void Node::set_global_scale(glm::vec3 scale) {
    Transform global = get_global_transform();
    glm::vec3 delta_scale = scale / global.scale;

    m_local_transform.scale = m_local_transform.scale * delta_scale;
}
