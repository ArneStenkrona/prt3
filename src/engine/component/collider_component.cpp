#include "collider_component.h"

#include "src/engine/scene/scene.h"

using namespace prt3;

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();

    Sphere sphere{};
    sphere.radius = 1.0f;
    m_tag = sys.add_sphere_collider(
        m_node_id,
        ColliderType::collider,
        sphere,
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    ColliderType type,
    ModelHandle model_handle
)
 : m_node_id{node_id} {
    Model const & model = scene.get_model(model_handle);
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_mesh_collider(
        m_node_id,
        type,
        model,
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    ColliderType type,
    Model const & model
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_mesh_collider(
        m_node_id,
        type,
        model,
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    ColliderType type,
    std::vector<glm::vec3> && triangles
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_mesh_collider(
        m_node_id,
        type,
        std::move(triangles),
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    ColliderType type,
    std::vector<glm::vec3> const & triangles
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_mesh_collider(
        m_node_id,
        type,
        triangles,
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    ColliderType type,
    Sphere const & sphere
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_sphere_collider(
        m_node_id,
        type,
        sphere,
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    ColliderType type,
    Box const & box
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_box_collider(
        m_node_id,
        type,
        box.dimensions,
        box.center,
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    std::istream & in
)
 : m_node_id{node_id} {
    deserialize(in, scene);
}

void ColliderComponent::set_collider(
    Scene & scene,
    ColliderType type,
    ModelHandle model_handle
) {
    Model const & model = scene.get_model(model_handle);
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_mesh_collider(
        m_node_id,
        type,
        model,
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::set_collider(
    Scene & scene,
    ColliderType type,
    Model const & model
) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_mesh_collider(
        m_node_id,
        type,
        model,
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::set_collider(
    Scene & scene,
    ColliderType type,
    std::vector<glm::vec3> && triangles
) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_mesh_collider(
        m_node_id,
        type,
        std::move(triangles),
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::set_collider(
    Scene & scene,
    ColliderType type,
    std::vector<glm::vec3> const & triangles
) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_mesh_collider(
        m_node_id,
        type,
        triangles,
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::set_collider(
    Scene & scene,
    ColliderType type,
    Sphere const & sphere
) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_sphere_collider(
        m_node_id,
        type,
        sphere,
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::set_collider(
    Scene & scene,
    ColliderType type,
    Box const & box
) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_box_collider(
        m_node_id,
        type,
        box.dimensions,
        box.center,
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    PhysicsSystem const & sys = scene.physics_system();

    write_stream(out, m_tag.shape);
    write_stream(out, m_tag.type);
    write_stream(out, sys.get_collision_layer(m_tag));
    write_stream(out, sys.get_collision_mask(m_tag));

    switch (m_tag.shape) {
        case ColliderShape::mesh: {
            MeshCollider const & col = sys.get_mesh_collider(m_tag.id, m_tag.type);
            auto const & tris = col.triangles();
            write_stream(out, tris.size());
            for (auto const & tri : tris) {
                write_stream(out, tri);
            }
            break;
        }
        case ColliderShape::sphere: {
            SphereCollider const & col = sys.get_sphere_collider(m_tag.id, m_tag.type);
            out << col.base_shape();
            break;
        }
        case ColliderShape::box: {
            BoxCollider const & col = sys.get_box_collider(m_tag.id, m_tag.type);
            write_stream(out, col.dimensions());
            write_stream(out, col.center());
            break;
        }
        default: {}
    }
}

void ColliderComponent::deserialize(
    std::istream & in,
    Scene & scene
) {
    if (m_tag.shape != ColliderShape::none) {
        remove(scene);
    }

    PhysicsSystem & sys = scene.physics_system();

    ColliderShape shape;
    read_stream(in, shape);
    ColliderType type;
    read_stream(in, type);
    CollisionLayer layer;
    CollisionLayer mask;
    read_stream(in, layer);
    read_stream(in, mask);

    switch (shape) {
        case ColliderShape::mesh: {
            std::vector<glm::vec3> tris;
            size_t n_tris;
            read_stream(in, n_tris);

            tris.resize(n_tris);
            for (glm::vec3 & vert : tris) {
                read_stream(in, vert);
            }

            m_tag = sys.add_mesh_collider(
                m_node_id,
                type,
                std::move(tris),
                scene.get_node(m_node_id).get_global_transform(scene)
            );
            break;
        }
        case ColliderShape::sphere: {
            Sphere sphere;
            in >> sphere;
            m_tag = sys.add_sphere_collider(
                m_node_id,
                type,
                sphere,
                scene.get_node(m_node_id).get_global_transform(scene)
            );
            break;
        }
        case ColliderShape::box: {
            glm::vec3 dimensions;
            glm::vec3 center;
            read_stream(in, dimensions);
            read_stream(in, center);
            m_tag = sys.add_box_collider(
                m_node_id,
                type,
                dimensions,
                center,
                scene.get_node(m_node_id).get_global_transform(scene)
            );
            break;
        }
        default: {}
    }

    sys.set_collision_layer(m_tag, layer);
    sys.set_collision_mask(m_tag, mask);
}

void ColliderComponent::remove(Scene & scene) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
}
