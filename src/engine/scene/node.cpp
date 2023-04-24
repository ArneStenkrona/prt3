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

Transform Node::get_inherited_transform(Scene const & scene) const {
    NodeID curr_id = m_parent_id;

    Transform tform;

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
    Transform inherit = get_inherited_transform(scene);
    glm::mat4 inv = glm::inverse(inherit.to_matrix());

    m_local_transform.position = inv * glm::vec4(transform.position, 1.0f);
    m_local_transform.scale = inv * glm::vec4(transform.scale, 0.0f);
    m_local_transform.rotation = transform.rotation * glm::quat_cast(inv);
}

void Node::set_global_position(
    Scene const & scene,
    glm::vec3 const & position
) {
    Transform inherit = get_inherited_transform(scene);
    glm::mat4 inv = glm::inverse(inherit.to_matrix());
    m_local_transform.position = inv * glm::vec4(position, 1.0f);
}

void Node::set_global_rotation(
    Scene const & scene,
    glm::quat const & rotation
) {
    Transform inherit = get_inherited_transform(scene);
    glm::mat4 inv = glm::inverse(inherit.to_matrix());
    m_local_transform.rotation = rotation * glm::quat_cast(inv);
}

void Node::set_global_scale(
    Scene const & scene,
    glm::vec3 scale
) {
    Transform inherit = get_inherited_transform(scene);
    glm::mat4 inv = glm::inverse(inherit.to_matrix());
    m_local_transform.scale = inv * glm::vec4(scale, 0.0f);
}

void Node::transform_node(Scene const & scene, Transform const & transform) {
    Transform new_tform = get_global_transform(scene);
    Transform inherit = get_inherited_transform(scene);
    glm::mat4 inv = glm::inverse(inherit.to_matrix());

    new_tform.position = new_tform.position + transform.position;
    new_tform.scale = new_tform.scale * transform.scale;
    new_tform.rotation = new_tform.rotation * transform.rotation;

    m_local_transform.position = inv * glm::vec4(new_tform.position, 1.0f);
    m_local_transform.scale = inv * glm::vec4(new_tform.scale, 0.0f);
    m_local_transform.rotation = new_tform.rotation * glm::quat_cast(inv);
}

void Node::translate_node(Scene const & scene, glm::vec3 const & translation) {
    Transform global = get_global_transform(scene);
    Transform inherit = get_inherited_transform(scene);
    glm::mat4 inv = glm::inverse(inherit.to_matrix());
    glm::vec3 position = global.position + translation;
    m_local_transform.position = inv * glm::vec4(position, 1.0f);
}

void Node::rotate_node(Scene const & scene, glm::quat const & rotation) {
    Transform global = get_global_transform(scene);
    Transform inherit = get_inherited_transform(scene);
    glm::mat4 inv = glm::inverse(inherit.to_matrix());
    glm::quat new_rotation = global.rotation * rotation;
    m_local_transform.rotation = new_rotation * glm::quat_cast(inv);
}

void Node::scale_node(Scene const & scene, glm::vec3 scale) {
    Transform global = get_global_transform(scene);
    Transform inherit = get_inherited_transform(scene);
    glm::mat4 inv = glm::inverse(inherit.to_matrix());
    glm::vec3 new_scale = global.scale * scale;
    m_local_transform.scale = inv * glm::vec4(new_scale, 0.0f);
}

Transform Node::global_to_local_transform(
    Scene const & scene,
    Transform const & transform
) const {
    Transform local;

    Transform inherit = get_inherited_transform(scene);
    glm::mat4 inv = glm::inverse(inherit.to_matrix());

    local.position = inv * glm::vec4(transform.position, 1.0f);
    local.scale = inv * glm::vec4(transform.scale, 0.0f);
    local.rotation = transform.rotation * glm::quat_cast(inv);

    return local;
}

CollisionResult Node::move_and_collide(
    Scene & scene,
    glm::vec3 const & movement
) {
    return scene.m_physics_system.move_and_collide(scene, m_id, movement);
}
