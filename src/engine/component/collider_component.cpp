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
        sphere,
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    ModelHandle model_handle
)
 : m_node_id{node_id} {
    Model const & model = scene.get_model(model_handle);
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_mesh_collider(
        m_node_id,
        model,
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    Model const & model
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_mesh_collider(
        m_node_id,
        model,
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    std::vector<glm::vec3> && triangles
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_mesh_collider(
        m_node_id,
        std::move(triangles),
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    std::vector<glm::vec3> const & triangles
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_mesh_collider(
        m_node_id,
        triangles,
        scene.get_node(node_id).get_global_transform(scene)
    );
}

ColliderComponent::ColliderComponent(
    Scene & scene,
    NodeID node_id,
    Sphere const & sphere
)
 : m_node_id{node_id} {
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_sphere_collider(
        m_node_id,
        sphere,
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
    ModelHandle model_handle
) {
    Model const & model = scene.get_model(model_handle);
    PhysicsSystem & sys = scene.physics_system();
    m_tag = sys.add_mesh_collider(
        m_node_id,
        model,
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::set_collider(
    Scene & scene,
    Model const & model
) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_mesh_collider(
        m_node_id,
        model,
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::set_collider(
    Scene & scene,
    std::vector<glm::vec3> && triangles
) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_mesh_collider(
        m_node_id,
        std::move(triangles),
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::set_collider(
    Scene & scene,
    std::vector<glm::vec3> const & triangles
) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_mesh_collider(
        m_node_id,
        triangles,
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::set_collider(
    Scene & scene,
    Sphere const & sphere
) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_sphere_collider(
        m_node_id,
        sphere,
        scene.get_node(m_node_id).get_global_transform(scene)
    );
}

void ColliderComponent::set_collider(
    Scene & scene,
    Box const & box
) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
    m_tag = sys.add_box_collider(
        m_node_id,
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

    write_stream(out, m_tag.type);
    switch (m_tag.type) {
        case ColliderType::collider_type_mesh: {
            MeshCollider const & col = sys.get_mesh_collider(m_tag.id);
            auto const & tris = col.triangles();
            write_stream(out, tris.size());
            for (auto const & tri : tris) {
                write_stream(out, tri);
            }
            break;
        }
        case ColliderType::collider_type_sphere: {
            SphereCollider const & col = sys.get_sphere_collider(m_tag.id);
            out << col.base_shape();
            break;
        }
        case ColliderType::collider_type_box: {
            BoxCollider const & col = sys.get_box_collider(m_tag.id);
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
    if (m_tag.type != ColliderType::collider_type_none) {
        remove(scene);
    }

    PhysicsSystem & sys = scene.physics_system();

    ColliderType type;
    read_stream(in, type);
    switch (type) {
        case ColliderType::collider_type_mesh: {
            std::vector<glm::vec3> tris;
            size_t n_tris;
            read_stream(in, n_tris);

            tris.resize(n_tris);
            for (glm::vec3 & vert : tris) {
                read_stream(in, vert);
            }

            m_tag = sys.add_mesh_collider(
                m_node_id,
                std::move(tris),
                scene.get_node(m_node_id).get_global_transform(scene)
            );
            break;
        }
        case ColliderType::collider_type_sphere: {
            Sphere sphere;
            in >> sphere;
            m_tag = sys.add_sphere_collider(
                m_node_id,
                sphere,
                scene.get_node(m_node_id).get_global_transform(scene)
            );
            break;
        }
        case ColliderType::collider_type_box: {
            glm::vec3 dimensions;
            glm::vec3 center;
            read_stream(in, dimensions);
            read_stream(in, center);
            m_tag = sys.add_box_collider(
                m_node_id,
                dimensions,
                center,
                scene.get_node(m_node_id).get_global_transform(scene)
            );
            break;
        }
        default: {}
    }
}

void ColliderComponent::remove(Scene & scene) {
    PhysicsSystem & sys = scene.physics_system();
    sys.remove_collider(m_tag);
}
