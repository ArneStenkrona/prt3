#include "node.h"

#include "src/engine/scene/scene.h"

#include <glm/gtx/matrix_decompose.hpp>

using namespace prt3;

Node::Node(NodeID id)
 : m_id{id} {

}

Transform Node::get_global_transform(Scene const & scene) const {
    NodeID curr_id = m_parent_id;

    Transform tform;
    tform.position = m_local_transform.position;
    tform.scale = m_local_transform.scale;
    tform.rotation = m_local_transform.rotation;

    while (curr_id != NO_NODE) {
        Node const & curr = scene.get_node(curr_id);
        Transform const & curr_tform = curr.local_transform();
        tform.position = curr_tform.position +
            glm::rotate(
                curr_tform.rotation,
                curr_tform.scale * tform.position
            );
        tform.scale = tform.scale * curr_tform.scale;
        tform.rotation = tform.rotation * curr_tform.rotation;
        curr_id = curr.m_parent_id;
    }

    return tform;
}

void Node::set_global_transform(
    Scene const & scene,
    Transform const & transform
) {
    Transform global = get_global_transform(scene);
    glm::vec3 delta_pos = transform.position - global.position;
    glm::vec3 delta_scale = transform.scale / global.scale;
    glm::quat delta_rot = transform.rotation
        * glm::inverse(global.rotation);

    m_local_transform.position = m_local_transform.position + delta_pos;
    m_local_transform.scale = m_local_transform.scale * delta_scale;
    m_local_transform.rotation = delta_rot * m_local_transform.rotation;
}

void Node::set_global_position(
    Scene const & scene,
    glm::vec3 const & position
) {
    NodeID curr_id = m_parent_id;

    glm::vec3 pos_p{0.0f};

    while (curr_id != NO_NODE) {
        Node const & curr = scene.get_node(curr_id);
        Transform const & curr_tform = curr.local_transform();
        pos_p = curr_tform.position +
            glm::rotate(
                curr_tform.rotation,
                curr_tform.scale * pos_p
            );
        curr_id = curr.m_parent_id;
    }

    m_local_transform.position = position - pos_p;
}

void Node::set_global_rotation(
    Scene const & scene,
    glm::quat const & rotation
) {
    NodeID curr_id = m_parent_id;

    glm::quat rot_p{1.0f, 0.0f, 0.0f, 0.0f};

    while (curr_id != NO_NODE) {
        Node const & curr = scene.get_node(curr_id);
        rot_p = rot_p * curr.local_transform().rotation;

        curr_id = curr.m_parent_id;
    }

    m_local_transform.rotation = rotation * glm::inverse(rot_p);
}

void Node::set_global_scale(
    Scene const & scene,
    glm::vec3 scale
) {
    NodeID curr_id = m_parent_id;

    glm::vec3 scale_p{1.0f};

    while (curr_id != NO_NODE) {
        Node const & curr = scene.get_node(curr_id);
        scale_p = scale_p * curr.local_transform().scale;

        curr_id = curr.m_parent_id;
    }

    m_local_transform.scale = scale / scale_p;
}

Collision Node::move_and_collide(
    Scene & scene,
    glm::vec3 const & movement
) {
    return scene.m_physics_system.move_and_collide(scene, m_id, movement);
}
