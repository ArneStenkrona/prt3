#include "physics_system.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

PhysicsSystem::PhysicsSystem(Scene & scene)
 : m_scene{scene} {}

void PhysicsSystem::add_mesh_collider(NodeID node_id,
                                      Model const & model) {
    ColliderTag tag = create_collider_from_model(model);
    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;
}

void PhysicsSystem::add_sphere_collider(NodeID node_id,
                                        Sphere const & sphere) {
    ColliderTag tag = create_sphere_collider(sphere);
    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;
}

collision_util::CollisionResult PhysicsSystem::move_and_collide(
    NodeID node_id,
    glm::vec3 const & movement) {
    collision_util::CollisionResult res{};

    ColliderTag const & tag = m_tags[node_id];
    Node & node = m_scene.get_node(node_id);

    Transform transform = node.get_global_transform();

    switch (tag.type) {
        case ColliderType::collider_type_sphere: {
            SphereCollider const & col = m_sphere_colliders[tag];
            res = move_and_collide(tag, col, movement, transform);
            break;
        }
        default: {}
    }
    node.set_global_transform(transform);
    return res;
}

ColliderTag PhysicsSystem::create_collider_from_model(
    Model const & model) {

    std::vector<Model::Vertex> v_buf = model.vertex_buffer();
    std::vector<uint32_t> i_buf = model.index_buffer();

    std::vector<Triangle> tris;
    tris.resize(i_buf.size() / 3);

    for (size_t i = 0; i < tris.size(); ++i) {
        size_t ii = 3 * i;
        tris[i].a = v_buf[i_buf[ii]].position;
        tris[i].b = v_buf[i_buf[ii + 1]].position;
        tris[i].c = v_buf[i_buf[ii + 2]].position;
    }

    ColliderTag tag;
    tag.id = m_next_mesh_id;
    ++m_next_mesh_id;
    tag.type = ColliderType::collider_type_mesh;

    MeshCollider & col = m_mesh_colliders[tag];
    col.set_triangles(std::move(tris));

    return tag;
}

ColliderTag PhysicsSystem::create_sphere_collider(Sphere const & sphere) {
    ColliderTag tag;
    tag.id = m_next_sphere_id;
    ++m_next_sphere_id;
    tag.type = ColliderType::collider_type_sphere;

    m_sphere_colliders[tag] = SphereCollider{sphere};

    return tag;
}

Node & PhysicsSystem::get_node(NodeID node_id) {
    return m_scene.get_node(node_id);
}
