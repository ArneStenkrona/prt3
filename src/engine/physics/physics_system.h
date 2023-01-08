#ifndef PRT3_PHYSICS_SYSTEM_H
#define PRT3_PHYSICS_SYSTEM_H

#include "src/engine/physics/gjk.h"

#include "src/engine/physics/aabb_tree.h"
#include "src/engine/physics/collider.h"
#include "src/engine/geometry/shapes.h"
#include "src/engine/rendering/model.h"
#include "src/engine/component/transform.h"
#include "src/engine/scene/node.h"
#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/renderer.h"

#include <unordered_map>
#include <cstdint>
#include <utility>

namespace prt3 {

class PhysicsSystem {
public:
    CollisionResult move_and_collide(
        Scene & scene,
        NodeID node_id,
        glm::vec3 const & movement
    );

    MeshCollider const & get_mesh_collider(ColliderID id) const
    { return m_mesh_colliders.at(id); }

    SphereCollider const & get_sphere_collider(ColliderID id) const
    { return m_sphere_colliders.at(id); }

    SphereCollider & get_sphere_collider(ColliderID id)
    { return m_sphere_colliders.at(id); }

    void collect_collider_render_data(
        Renderer & renderer,
        Transform const * transforms,
        NodeID selected,
        ColliderRenderData & data
    );

private:
    std::unordered_map<NodeID, ColliderTag> m_tags;
    std::unordered_map<ColliderTag, NodeID> m_node_ids;

    std::unordered_map<ColliderID, MeshCollider> m_mesh_colliders;
    ColliderID m_next_mesh_id = 0;
    std::unordered_map<ColliderID, SphereCollider> m_sphere_colliders;
    ColliderID m_next_sphere_id = 0;

    DynamicAABBTree m_aabb_tree;

    std::unordered_map<ColliderTag, ResourceID> m_collider_meshes;

    void clear();

    void update(
        Transform const * transforms,
        Transform const * transforms_history
    );

    ColliderTag add_mesh_collider(
        NodeID node_id,
        std::vector<glm::vec3> && triangles,
        Transform const & transform
    );

    ColliderTag add_mesh_collider(
        NodeID node_id,
        std::vector<glm::vec3> const & triangles,
        Transform const & transform
    );

    ColliderTag add_mesh_collider(
        NodeID node_id,
        Model const & model,
        Transform const & transform
    );

    // ColliderTag add_mesh_collider(
    //     NodeID node_id,
    //     Model const & model,
    //     uint32_t mesh_index,
    //     Transform const & transform
    // );

    ColliderTag add_sphere_collider(
        NodeID node_id,
        Sphere const & sphere,
        Transform const & transform
    );

    ColliderTag create_collider_from_triangles(
        std::vector<glm::vec3> && triangles,
        Transform const & transform
    );

    ColliderTag create_collider_from_triangles(
        std::vector<glm::vec3> const & triangles,
        Transform const & transform
    );

    ColliderTag create_collider_from_model(
        Model const & model,
        Transform const & transform
    );

    ColliderTag create_sphere_collider(
        Sphere const & sphere,
        Transform const & transform
    );

    void remove_collider(ColliderTag tag);

    Node & get_node(Scene & scene, NodeID node_id);
    Transform get_global_transform(Scene & scene, NodeID node_id);

    template<typename Collider, typename Shape>
    CollisionResult move_and_collide(
        Scene & scene,
        ColliderTag tag,
        Collider const & collider,
        glm::vec3 const & movement,
        Transform & transform
    ) {
        unsigned int iteration = 0;

        CollisionResult ret{};

        glm::vec3 curr_movement = movement;
        while (iteration < MAX_COLLISION_ITER) {
            Collision & collision = ret.collisions[ret.n_collisions];

            AABB aabb = calculate_aabb(collider, transform, curr_movement);

            Transform start_transform = transform;
            transform.position += curr_movement;

            thread_local std::array<std::vector<ColliderID>,
                ColliderType::collider_type_none> candidates;

            ColliderTag closest_candidate;
            float t = std::numeric_limits<float>::max();
            closest_candidate.type = ColliderType::collider_type_none;
            Sphere sphere_this;
            GJKRes gjk_res;
            Triangle tri_this;
            Sphere sphere_other;
            Triangle tri_other;

            // clear thread_local candidate buffer
            for (auto & candidate : candidates) {
                candidate.resize(0);
            }
            m_aabb_tree.query(tag, aabb, candidates);

            for (unsigned int type_i = 0;
                type_i < ColliderType::collider_type_none;
                ++type_i
            ) {
                ColliderType type =  static_cast<ColliderType>(type_i);
                std::vector<ColliderID> const & ids = candidates[type];

                switch (type) {
                    case ColliderType::collider_type_sphere: {
                        inner_collide<Collider, SphereCollider>(
                            scene,
                            collider,
                            start_transform,
                            curr_movement,
                            ids,
                            closest_candidate,
                            t,
                            gjk_res,
                            tri_this,
                            sphere_other,
                            tri_other
                        );
                        break;
                    }
                    case ColliderType::collider_type_mesh: {
                        inner_collide<Collider, MeshCollider>(
                            scene,
                            collider,
                            start_transform,
                            curr_movement,
                            ids,
                            closest_candidate,
                            t,
                            gjk_res,
                            tri_this,
                            sphere_other,
                            tri_other
                        );
                        break;
                    }
                    default: {}
                }
            }

            collision.collided =
                closest_candidate.type != ColliderType::collider_type_none;

            switch (closest_candidate.type) {
                case ColliderType::collider_type_sphere: {
                    auto swept = get_swept_shape<Collider, Shape>(
                        start_transform,
                        curr_movement * gjk_res.t,
                        collider,
                        tri_this
                    );

                    EPARes epa_res = epa(
                        gjk_res.simplex,
                        gjk_res.n_simplex,
                        swept,
                        sphere_other
                    );

                    collision.normal = epa_res.normal;
                    collision.impulse = epa_res.normal * epa_res.penetration_depth;

                    break;
                }
                case ColliderType::collider_type_mesh: {
                    auto swept = get_swept_shape<Collider, Shape>(
                        start_transform,
                        curr_movement * gjk_res.t,
                        collider,
                        tri_this
                    );

                    EPARes epa_res = epa(
                        gjk_res.simplex,
                        gjk_res.n_simplex,
                        swept,
                        tri_other
                    );

                    collision.normal = epa_res.normal;
                    collision.impulse = epa_res.normal * epa_res.penetration_depth;

                    break;
                }
                default: {
                    // No collision
                }
            }

            if (collision.collided) {
                // check if grounded
                if (glm::dot(collision.normal, glm::vec3{0.0f, 1.0f, 0.0f}) > 0.707) {
                    ret.grounded = true;
                    ret.ground_normal = collision.normal;
                }

                // resolve collision
                transform.position =
                    start_transform.position + t * curr_movement + collision.impulse;
                glm::vec3 remain = (1.0f - t) * curr_movement;
                curr_movement =
                    remain - glm::dot(remain, collision.normal) * collision.normal;

                ++ret.n_collisions;
            } else {
                break;
            }

            ++iteration;
        }
        return ret;
    }

    template<typename Collider, typename Other>
    void inner_collide(
        Scene & scene,
        Collider const & collider,
        Transform const & transform,
        glm::vec3 const & curr_movement,
        std::vector<ColliderID> const & ids,
        ColliderTag & closest_candidate,
        float & t,
        GJKRes & gjk_res,
        Triangle & tri_this,
        Sphere & sphere_other,
        Triangle & tri_other
    );

    template<>
    void inner_collide<SphereCollider, SphereCollider>(
        Scene & scene,
        SphereCollider const & collider,
        Transform const & transform,
        glm::vec3 const & curr_movement,
        std::vector<ColliderID> const & ids,
        ColliderTag & closest_candidate,
        float & t,
        GJKRes & gjk_res,
        Triangle & /*tri_this*/,
        Sphere & sphere_other,
        Triangle & /*tri_other*/
    ) {
        Sphere shape = collider.get_shape(transform);

        for (ColliderID id : ids) {
            ColliderTag tag_other;
            tag_other.id = id;
            tag_other.type = ColliderType::collider_type_sphere;

            NodeID other_node_id = m_node_ids.at(tag_other);
            Transform transform_other = get_global_transform(scene, other_node_id);

            Sphere shape_other =
                m_sphere_colliders.at(id).get_shape(transform_other);

            GJKRes gjk_cand =
                find_collision_gjk(
                    shape,
                    shape_other,
                    curr_movement
                );

            if (gjk_cand.collided && gjk_cand.t < t) {
                closest_candidate = tag_other;
                t = gjk_cand.t;
                gjk_res = gjk_cand;
                sphere_other = shape_other;
            }
        }
    }

    template<>
    void inner_collide<SphereCollider, MeshCollider>(
        Scene & /*scene*/,
        SphereCollider const & collider,
        Transform const & transform,
        glm::vec3 const & curr_movement,
        std::vector<ColliderID> const & ids,
        ColliderTag & closest_candidate,
        float & t,
        GJKRes & gjk_res,
        Triangle & /*tri_this*/,
        Sphere & /*sphere_other*/,
        Triangle & tri_other
    ) {
        Sphere shape = collider.get_shape(transform);
        SweptSphere swept_shape = shape.sweep(curr_movement);
        AABB aabb = swept_shape.aabb();

        for (ColliderID id : ids) {
            ColliderTag tag_other;
            tag_other.id = id;
            tag_other.type = ColliderType::collider_type_mesh;

            MeshCollider const & mesh_collider = m_mesh_colliders[id];

            thread_local std::vector<Triangle> tris;
            tris.resize(0);
            mesh_collider.collect_triangles(
                aabb,
                tris
            );
            for (Triangle tri : tris) {
                GJKRes gjk_cand =
                    find_collision_gjk(
                        shape,
                        tri,
                        curr_movement
                    );
                if (gjk_cand.collided && gjk_cand.t < t) {
                    closest_candidate = tag_other;
                    t = gjk_cand.t;
                    gjk_res = gjk_cand;
                    tri_other = tri;
                }
            }
        }
    }

    template<>
    void inner_collide<MeshCollider, SphereCollider>(
        Scene & scene,
        MeshCollider const & collider,
        Transform const & /*transform*/,
        glm::vec3 const & curr_movement,
        std::vector<ColliderID> const & ids,
        ColliderTag & closest_candidate,
        float & t,
        GJKRes & gjk_res,
        Triangle & tri_this,
        Sphere & sphere_other,
        Triangle & /*tri_other*/
    ) {
        for (size_t i = 0; i < collider.triangle_cache().size(); i += 3) {
            Triangle triangle;
            triangle.a = collider.triangle_cache()[i];
            triangle.b = collider.triangle_cache()[i+1];
            triangle.c = collider.triangle_cache()[i+2];

            for (ColliderID id : ids) {
                ColliderTag tag_other;
                tag_other.id = id;
                tag_other.type = ColliderType::collider_type_sphere;

                NodeID other_node_id = m_node_ids.at(tag_other);
                Transform transform_other = get_global_transform(scene, other_node_id);

                Sphere shape_other =
                    m_sphere_colliders.at(id).get_shape(transform_other);

                GJKRes gjk_cand =
                    find_collision_gjk(
                        triangle,
                        shape_other,
                        curr_movement
                    );

                if (gjk_cand.collided && gjk_cand.t < t) {
                    closest_candidate = tag_other;
                    t = gjk_cand.t;
                    gjk_res = gjk_cand;
                    tri_this = triangle;
                    sphere_other = shape_other;
                }
            }
        }
    }

    template<>
    void inner_collide<MeshCollider, MeshCollider>(
        Scene & /*scene*/,
        MeshCollider const & collider,
        Transform const & /*transform*/,
        glm::vec3 const & curr_movement,
        std::vector<ColliderID> const & ids,
        ColliderTag & closest_candidate,
        float & t,
        GJKRes & gjk_res,
        Triangle & tri_this,
        Sphere & /*sphere_other*/,
        Triangle & tri_other
    ) {
        // TODO: Use acceleration structure to avoid O(M*N)
        for (size_t i = 0; i < collider.triangle_cache().size(); i += 3) {
            Triangle triangle;
            triangle.a = collider.triangle_cache()[i];
            triangle.b = collider.triangle_cache()[i+1];
            triangle.c = collider.triangle_cache()[i+2];

            AABB aabb = triangle.sweep(curr_movement).aabb();

            for (ColliderID id : ids) {
                ColliderTag tag_other;
                tag_other.id = id;
                tag_other.type = ColliderType::collider_type_mesh;

                MeshCollider const & mesh_collider = m_mesh_colliders[id];

                thread_local std::vector<Triangle> tris;
                tris.resize(0);
                mesh_collider.collect_triangles(
                    aabb,
                    tris
                );
                for (Triangle tri : tris) {
                    GJKRes gjk_cand =
                        find_collision_gjk(
                            triangle,
                            tri,
                            curr_movement
                        );
                    if (gjk_cand.collided && gjk_cand.t < t) {
                        closest_candidate = tag_other;
                        t = gjk_cand.t;
                        gjk_res = gjk_cand;
                        tri_this = triangle;
                        tri_other = tri;
                    }
                }
            }
        }
    }

    template<typename Collider>
    AABB calculate_aabb(
        Collider const & collider,
        Transform const & transform,
        glm::vec3 const & curr_movement
    ) const;

    template<>
    AABB calculate_aabb<SphereCollider>(
        SphereCollider const & collider,
        Transform const & transform,
        glm::vec3 const & curr_movement
        ) const {
        return collider.get_shape(transform).sweep(curr_movement).aabb();
    }

    template<>
    AABB calculate_aabb<MeshCollider>(
        MeshCollider const & collider,
        Transform const & /*transform*/,
        glm::vec3 const & curr_movement
    ) const {
        return collider.aabb().sweep(curr_movement);
    }

    template<typename Collider, typename Shape>
    decltype(std::declval<Shape>().sweep({})) get_swept_shape(
        Transform const & transform,
        glm::vec3 const & curr_movement,
        Collider const & collider,
        Triangle const & triangle
    ) const;

    template<>
    SweptSphere get_swept_shape<SphereCollider, Sphere>(
        Transform const & transform,
        glm::vec3 const & curr_movement,
        SphereCollider const & collider,
        Triangle const & /*triangle*/
    ) const {
        return collider.get_shape(transform).sweep(curr_movement);
    }

    template<>
    SweptTriangle get_swept_shape<MeshCollider, Triangle>(
        Transform const & /*transform*/,
        glm::vec3 const & curr_movement,
        MeshCollider const & /*collider*/,
        Triangle const & triangle
    ) const {
        return triangle.sweep(curr_movement);
    }

    friend class Scene;
    friend class ColliderComponent;
};

} // namespace prt3

#endif
