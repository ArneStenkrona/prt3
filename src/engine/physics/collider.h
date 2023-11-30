#ifndef PRT3_COLLIDER_H
#define PRT3_COLLIDER_H

#include "src/engine/physics/aabb.h"
#include "src/engine/physics/collider_tag.h"
#include "src/engine/physics/aabb_tree.h"
#include "src/engine/geometry/shapes.h"

#include <glm/gtx/component_wise.hpp>

#include <vector>

namespace prt3 {

class PhysicsSystem;

struct Collision {
    glm::vec3 normal;
    glm::vec3 impulse;
    ColliderTag other;
    bool collided = false;
};

constexpr size_t MAX_COLLISION_ITER = 10;

struct CollisionResult {
    std::array<Collision, MAX_COLLISION_ITER> collisions;
    unsigned int n_collisions;
    bool grounded = false;
    glm::vec3 ground_normal;
    float move_distance;
};

class MeshCollider {
public:
    static constexpr ColliderShape shape = ColliderShape::mesh;

    void set_triangles(std::vector<glm::vec3> && triangles);
    void set_triangles(std::vector<glm::vec3> const & triangles);
    void set_triangles(std::vector<Triangle> && triangles);
    void collect_triangles(AABB const & aabb,
                           std::vector<Triangle> & triangles) const;
    void set_transform(Transform const & transform);
    AABB const & aabb() const { return m_aabb; };

    std::vector<glm::vec3> const & triangles() const { return m_triangles; }
    std::vector<glm::vec3> const & triangle_cache() const { return m_triangle_cache; }

    CollisionLayer get_layer() const { return m_layer; }
    void set_layer(CollisionLayer layer)
    { m_layer = layer; m_layer_changed = true; }
    CollisionLayer get_mask() const { return m_mask; }
    void set_mask(CollisionLayer mask)
    { m_mask = mask; }

private:
    std::vector<glm::vec3> m_triangles;
    std::vector<glm::vec3> m_triangle_cache;
    DynamicAABBTree m_aabb_tree; // Consider a faster non-dynamic structure?
    Transform m_transform;
    AABB m_aabb;

    CollisionLayer m_layer;
    CollisionLayer m_mask;
    bool m_layer_changed = false;
    mutable bool m_changed = true;

    void update_triangle_cache();

    friend class PhysicsSystem;
};

class SphereCollider {
public:
    static constexpr ColliderShape shape = ColliderShape::sphere;

    SphereCollider() {}
    SphereCollider(Sphere const & sphere)
     : m_base_shape{sphere} {}

    Sphere get_shape(Transform const & transform) const {
        glm::mat4 mat = transform.to_matrix();
        float radius = m_base_shape.radius * glm::compMax(transform.scale);
        return { glm::vec3{mat * glm::vec4{m_base_shape.position, 1.0f}},
                 radius };
    }
    Sphere const & base_shape() const { return m_base_shape; }
    void set_base_shape(Sphere const & shape)
    { m_base_shape = shape; m_changed = true; }

    CollisionLayer get_layer() const { return m_layer; }
    void set_layer(CollisionLayer layer)
    { m_layer = layer; m_layer_changed = true; }
    CollisionLayer get_mask() const { return m_mask; }
    void set_mask(CollisionLayer mask)
    { m_mask = mask; }

private:
    Sphere m_base_shape;

    CollisionLayer m_layer;
    CollisionLayer m_mask;
    bool m_layer_changed = false;
    mutable bool m_changed = true;

    friend class PhysicsSystem;
};

struct Box {
    glm::vec3 dimensions;
    glm::vec3 center;
};

class BoxCollider {
public:
    static constexpr ColliderShape shape = ColliderShape::box;

    BoxCollider() {}
    BoxCollider(glm::vec3 dimensions, glm::vec3 center)
     : m_dimensions{dimensions}, m_center{center} {}
    BoxCollider(Box const & box)
     : m_dimensions{box.dimensions}, m_center{box.center} {}

    DiscreteConvexHull<8> get_shape(glm::mat4 const & mat) const {
        DiscreteConvexHull<8> shape;

        glm::vec3 max = m_center + 0.5f * m_dimensions;
        glm::vec3 min = m_center - 0.5f * m_dimensions;
        shape.vertices[0] = mat * (glm::vec4{min.x, min.y, min.z, 1.0f});
        shape.vertices[1] = mat * (glm::vec4{min.x, min.y, max.z, 1.0f});
        shape.vertices[2] = mat * (glm::vec4{min.x, max.y, min.z, 1.0f});
        shape.vertices[3] = mat * (glm::vec4{min.x, max.y, max.z, 1.0f});
        shape.vertices[4] = mat * (glm::vec4{max.x, min.y, min.z, 1.0f});
        shape.vertices[5] = mat * (glm::vec4{max.x, min.y, max.z, 1.0f});
        shape.vertices[6] = mat * (glm::vec4{max.x, max.y, min.z, 1.0f});
        shape.vertices[7] = mat * (glm::vec4{max.x, max.y, max.z, 1.0f});

        return shape;
    }

    DiscreteConvexHull<8> get_shape(Transform const & transform) const {
        return get_shape(transform.to_matrix());
    }

    Box base_shape() const {
        return Box{ m_dimensions, m_center };
    }

    glm::vec3 dimensions() const { return m_dimensions; }
    glm::vec3 center() const { return m_center; }

    void set_dimensions(glm::vec3 dimensions)
    { m_dimensions = dimensions; m_changed = true; }
    void set_center(glm::vec3 center)
    { m_center = center; m_changed = true; }

    CollisionLayer get_layer() const { return m_layer; }
    void set_layer(CollisionLayer layer)
    { m_layer = layer; m_layer_changed = true; }
    CollisionLayer get_mask() const { return m_mask; }
    void set_mask(CollisionLayer mask)
    { m_mask = mask; }

private:
    glm::vec3 m_dimensions;
    glm::vec3 m_center;

    CollisionLayer m_layer;
    CollisionLayer m_mask;
    bool m_layer_changed = false;
    mutable bool m_changed = true;

    friend class PhysicsSystem;
};

class CapsuleCollider {
public:
    static constexpr ColliderShape shape = ColliderShape::capsule;

    CapsuleCollider() {}
    CapsuleCollider(Capsule const & capsule)
     : m_base_shape{capsule} {}

    Capsule get_shape(Transform const & transform) const {
        glm::mat4 mat = transform.to_matrix();
        float radius = m_base_shape.radius * glm::compMax(transform.scale);
        return { glm::vec3{mat * glm::vec4{m_base_shape.start, 1.0f}},
                 glm::vec3{mat * glm::vec4{m_base_shape.end, 1.0f}},
                 radius };
    }
    Capsule const & base_shape() const { return m_base_shape; }
    void set_base_shape(Capsule const & shape)
    { m_base_shape = shape; m_changed = true; }

    CollisionLayer get_layer() const { return m_layer; }
    void set_layer(CollisionLayer layer)
    { m_layer = layer; m_layer_changed = true; }
    CollisionLayer get_mask() const { return m_mask; }
    void set_mask(CollisionLayer mask)
    { m_mask = mask; }

private:
    Capsule m_base_shape;

    CollisionLayer m_layer;
    CollisionLayer m_mask;
    bool m_layer_changed = false;
    mutable bool m_changed = true;

    friend class PhysicsSystem;
};

} // namespace prt3

#endif
