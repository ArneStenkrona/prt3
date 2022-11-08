#include "physics_system.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

ColliderTag PhysicsSystem::add_mesh_collider(
    NodeID node_id,
    std::vector<glm::vec3> && triangles,
    Transform const & transform
) {
    ColliderTag tag = create_collider_from_triangles(
        std::move(triangles),
        transform
    );

    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;

    return tag;
}

ColliderTag PhysicsSystem::add_mesh_collider(
    NodeID node_id,
    Model const & model,
    Transform const & transform
) {
    ColliderTag tag = create_collider_from_model(
        model,
        transform
    );

    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;

    return tag;
}

ColliderTag PhysicsSystem::add_sphere_collider(
    NodeID node_id,
    Sphere const & sphere,
    Transform const & transform
) {
    ColliderTag tag = create_sphere_collider(
        sphere,
        transform
    );
    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;

    return tag;
}

void PhysicsSystem::remove_collider(ColliderTag tag) {
    switch (tag.type) {
        case ColliderType::collider_type_mesh: {
            m_mesh_colliders.erase(tag.id);
            break;
        }
        case ColliderType::collider_type_sphere: {
            m_sphere_colliders.erase(tag.id);
            break;
        }
        case ColliderType::collider_type_none: {
            return;
        }
    }

    NodeID node_id = m_node_ids[tag];
    m_node_ids.erase(tag);
    m_tags.erase(node_id);

    m_aabb_tree.remove(tag);
}

Collision PhysicsSystem::move_and_collide(
    Scene & scene,
    NodeID node_id,
    glm::vec3 const & movement
) {
    Collision res{};

    ColliderTag const & tag = m_tags[node_id];
    Node & node = scene.get_node(node_id);

    Transform transform = node.get_global_transform(scene);
    Transform start_transform = transform;

    switch (tag.type) {
        case ColliderType::collider_type_sphere: {
            SphereCollider const & col = m_sphere_colliders[tag.id];
            res = move_and_collide(scene, tag, col, movement, transform);
            break;
        }
        // TODO: mesh
        default: {}
    }
    node.set_global_transform(scene, transform);

    if (transform != start_transform) {
        AABB aabb;
        switch (tag.type) {
            case ColliderType::collider_type_sphere: {
                aabb = m_sphere_colliders[tag.id].get_shape(transform).aabb();
                break;
            }
            // TODO: mesh
            default: {}
        }
        m_aabb_tree.update(tag, aabb);
    }
    return res;
}

ColliderTag PhysicsSystem::create_collider_from_triangles(
        std::vector<glm::vec3> && triangles,
        Transform const & transform
) {
    ColliderTag tag;
    tag.type = ColliderType::collider_type_mesh;
    tag.id = m_next_mesh_id;
    ++m_next_mesh_id;

    MeshCollider & col = m_mesh_colliders[tag.id];
    col.set_transform(transform);
    col.set_triangles(std::move(triangles));

    m_aabb_tree.insert(tag, col.aabb());

    return tag;
}

ColliderTag PhysicsSystem::create_collider_from_model(
    Model const & model,
    Transform const & transform
) {

    std::vector<Model::Vertex> v_buf = model.vertex_buffer();
    std::vector<uint32_t> i_buf = model.index_buffer();

    std::vector<glm::vec3> tris;
    tris.resize(i_buf.size());

    for (size_t i = 0; i < tris.size(); ++i) {
        tris[i] = v_buf[i_buf[i]].position;
    }

    ColliderTag tag;
    tag.type = ColliderType::collider_type_mesh;
    tag.id = m_next_mesh_id;
    ++m_next_mesh_id;

    MeshCollider & col = m_mesh_colliders[tag.id];
    col.set_transform(transform);
    col.set_triangles(std::move(tris));

    m_aabb_tree.insert(tag, col.aabb());

    return tag;
}

ColliderTag PhysicsSystem::create_sphere_collider(
    Sphere const & sphere,
    Transform const & transform) {
    ColliderTag tag;
    tag.type = ColliderType::collider_type_sphere;
    tag.id = m_next_sphere_id;
    ++m_next_sphere_id;

    m_sphere_colliders[tag.id] = SphereCollider{sphere};

    m_aabb_tree.insert(
        tag,
        m_sphere_colliders[tag.id].get_shape(transform).aabb()
    );

    return tag;
}

Node & PhysicsSystem::get_node(Scene & scene, NodeID node_id) {
    return scene.get_node(node_id);
}

Transform PhysicsSystem::get_global_transform(Scene & scene, NodeID node_id) {
    return scene.get_node(node_id).get_global_transform(scene);
}

void PhysicsSystem::clear() {
    m_tags.clear();
    m_node_ids.clear();

    m_mesh_colliders.clear();
    m_next_mesh_id = 0;
    m_sphere_colliders.clear();
    m_next_sphere_id = 0;

    m_aabb_tree.clear();
}

void PhysicsSystem::update(
    Transform const * transforms,
    Transform const * transforms_history
) {
    static std::vector<ColliderTag> stale_tags;
    stale_tags.resize(0);
    static std::vector<AABB> new_aabbs;
    new_aabbs.resize(0);

    for (auto const & pair : m_tags) {
        NodeID id = pair.first;
        ColliderTag tag = pair.second;
        Transform const & t_curr = transforms[id];
        Transform const & t_hist = transforms_history[id];

        // TODO: add tolerance to aabbs
        bool stale = t_curr != t_hist;

        if (stale) {
            stale_tags.push_back(tag);
            switch (tag.type) {
                case ColliderType::collider_type_sphere: {
                    AABB aabb = m_sphere_colliders[tag.id].get_shape(t_curr).aabb();
                    new_aabbs.push_back(aabb);
                    break;
                }
                case ColliderType::collider_type_mesh: {
                    MeshCollider & col = m_mesh_colliders[tag.id];
                    col.set_transform(t_curr);
                    AABB aabb = col.aabb();
                    new_aabbs.push_back(aabb);
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    m_aabb_tree.update(stale_tags.data(),
                       new_aabbs.data(),
                       new_aabbs.size());
}
