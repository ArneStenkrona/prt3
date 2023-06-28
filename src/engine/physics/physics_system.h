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
#include "src/engine/physics/collider_container.h"

#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <utility>
#include <functional>

namespace prt3 {

class Overlap {
public:
    Overlap(NodeID a, NodeID b) {
        if (a < b) {
            m_a = a;
            m_b = b;
        } else {
            m_a = b;
            m_b = a;
        }
    }

    NodeID a() const { return m_a; }
    NodeID b() const { return m_b; }

private:
    NodeID m_a;
    NodeID m_b;
};

inline bool operator==(Overlap const & lhs, Overlap const & rhs) {
    return lhs.a() == rhs.a() && lhs.b() == rhs.b();
}

inline bool operator!=(Overlap const & lhs, Overlap const & rhs) {
    return lhs.a() != rhs.a() || lhs.b() != rhs.b();
}

} // namespace prt3

namespace std {
  template <>
  struct hash<prt3::Overlap> {
    size_t operator()(prt3::Overlap const & o) const {
      return hash<prt3::NodeID>()(o.a()) ^
             (hash<prt3::NodeID>()(o.b()) << 1);
    }
  };
} // namespace std

namespace prt3 {

struct RayHit {
    glm::vec3 position;
    ColliderTag tag;
    NodeID node_id;
};

class PhysicsSystem {
public:
    CollisionResult move_and_collide(
        Scene & scene,
        NodeID node_id,
        glm::vec3 const & movement
    );

    bool raycast(
        glm::vec3 origin,
        glm::vec3 direction,
        float max_distance,
        CollisionLayer mask,
        ColliderTag ignore,
        RayHit & hit
    ) const;

    std::unordered_map<ColliderID, MeshCollider> const & mesh_colliders() const {
        return get_container(ColliderType::collider).meshes.map;
    }

    std::unordered_map<ColliderID, SphereCollider> const & sphere_colliders() const {
        return get_container(ColliderType::collider).spheres.map;
    }

    std::unordered_map<ColliderID, BoxCollider> const & box_colliders() const {
        return get_container(ColliderType::collider).boxes.map;
    }

    MeshCollider const & get_mesh_collider(ColliderID id, ColliderType type) const
    { return get_container(type).meshes.map.at(id); }

    SphereCollider const & get_sphere_collider(ColliderID id, ColliderType type) const
    { return get_container(type).spheres.map.at(id); }

    SphereCollider & get_sphere_collider(ColliderID id,  ColliderType type)
    { return get_container(type).spheres.map.at(id); }

    BoxCollider const & get_box_collider(ColliderID id,  ColliderType type) const
    { return get_container(type).boxes.map.at(id); }

    BoxCollider const & get_box_collider(ColliderID id,  ColliderType type)
    { return get_container(type).boxes.map.at(id); }

    CapsuleCollider const & get_capsule_collider(ColliderID id, ColliderType type) const
    { return get_container(type).capsules.map.at(id); }

    CapsuleCollider & get_capsule_collider(ColliderID id,  ColliderType type)
    { return get_container(type).capsules.map.at(id); }

    void set_collision_layer(ColliderTag tag, CollisionLayer layer) {
        switch (tag.shape) {
            case ColliderShape::mesh:
                get_container(tag.type).meshes.map.at(tag.id).set_layer(layer);
                break;
            case ColliderShape::sphere:
                get_container(tag.type).spheres.map.at(tag.id).set_layer(layer);
                break;
            case ColliderShape::box:
                get_container(tag.type).boxes.map.at(tag.id).set_layer(layer);
                break;
            case ColliderShape::capsule:
                get_container(tag.type).capsules.map.at(tag.id).set_layer(layer);
                break;
            default: assert(false);
        }
        get_container(tag.type).aabb_tree.set_layer(tag, layer);
    }

    CollisionLayer get_collision_layer(
        ColliderTag tag
    ) const {
        switch (tag.shape) {
            case ColliderShape::mesh:
                return get_container(tag.type).meshes.map.at(tag.id).get_layer();
            case ColliderShape::sphere:
                return get_container(tag.type).spheres.map.at(tag.id).get_layer();
            case ColliderShape::box:
                return get_container(tag.type).boxes.map.at(tag.id).get_layer();
            case ColliderShape::capsule:
                return get_container(tag.type).capsules.map.at(tag.id).get_layer();
            default: assert(false); return {};
        }
    }

    void set_collision_mask(ColliderTag tag, CollisionLayer mask) {
        switch (tag.shape) {
            case ColliderShape::mesh:
                get_container(tag.type).meshes.map.at(tag.id).set_mask(mask);
                break;
            case ColliderShape::sphere:
                get_container(tag.type).spheres.map.at(tag.id).set_mask(mask);
                break;
            case ColliderShape::box:
                get_container(tag.type).boxes.map.at(tag.id).set_mask(mask);
                break;
            case ColliderShape::capsule:
                get_container(tag.type).capsules.map.at(tag.id).set_mask(mask);
                break;
            default: assert(false);
        }
    }

    CollisionLayer get_collision_mask(
        ColliderTag tag
    ) const {
        switch (tag.shape) {
            case ColliderShape::mesh:
                return get_container(tag.type).meshes.map.at(tag.id).get_mask();
            case ColliderShape::sphere:
                return get_container(tag.type).spheres.map.at(tag.id).get_mask();
            case ColliderShape::box:
                return get_container(tag.type).boxes.map.at(tag.id).get_mask();
            case ColliderShape::capsule:
                return get_container(tag.type).capsules.map.at(tag.id).get_mask();
            default: assert(false); return {};
        }
    }

    void update_mesh_data(
        Renderer & renderer,
        ColliderType type
    );

    void update_sphere_data(
        Renderer & renderer,
        ColliderType type
    );

    void update_box_data(
        Renderer & renderer,
        ColliderType type
    );

    void update_capsule_data(
        Renderer & renderer,
        ColliderType type
    );

    void collect_render_data(
        Renderer & renderer,
        Transform const * transforms,
        NodeID selected,
        EditorRenderData & data
    );

    std::vector<NodeID> const & get_overlaps(NodeID node_id) const;

private:
    std::unordered_map<NodeID, ColliderTag> m_tags;
    std::unordered_map<ColliderTag, NodeID> m_node_ids;

    ColliderContainer m_colliders;
    ColliderContainer m_areas;

    std::unordered_map<ColliderTag, ResourceID> m_collider_meshes;

    std::unordered_map<NodeID, unsigned int> m_node_to_overlaps;
    std::vector<std::vector<NodeID> > m_overlaps;

    void clear();

    void update(
        Transform const * transforms,
        Transform const * transforms_history
    );

    void update_areas(
        Transform const * transforms,
        Transform const * transforms_history
    );

    ColliderContainer const & get_container(ColliderType type) const
    { return type == ColliderType::collider ? m_colliders : m_areas; }
    ColliderContainer & get_container(ColliderType type)
    { return type == ColliderType::collider ? m_colliders : m_areas; }

    ColliderTag add_mesh_collider(
        NodeID node_id,
        ColliderType type,
        std::vector<glm::vec3> && triangles,
        Transform const & transform
    );

    ColliderTag add_mesh_collider(
        NodeID node_id,
        ColliderType type,
        std::vector<glm::vec3> const & triangles,
        Transform const & transform
    );

    ColliderTag add_mesh_collider(
        NodeID node_id,
        ColliderType type,
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
        ColliderType type,
        Sphere const & sphere,
        Transform const & transform
    );

    ColliderTag add_box_collider(
        NodeID node_id,
        ColliderType type,
        glm::vec3 const & dimensions,
        glm::vec3 const & center,
        Transform const & transform
    );

    ColliderTag add_capsule_collider(
        NodeID node_id,
        ColliderType type,
        Capsule const & capsule,
        Transform const & transform
    );

    ColliderTag create_collider_from_triangles(
        std::vector<glm::vec3> && triangles,
        Transform const & transform,
        ColliderType type
    );

    ColliderTag create_collider_from_triangles(
        std::vector<glm::vec3> const & triangles,
        Transform const & transform,
        ColliderType type
    );

    ColliderTag create_collider_from_model(
        Model const & model,
        Transform const & transform,
        ColliderType type
    );

    ColliderTag create_sphere_collider(
        Sphere const & sphere,
        Transform const & transform,
        ColliderType type
    );

    ColliderTag create_box_collider(
        glm::vec3 dimensions,
        glm::vec3 center,
        Transform const & transform,
        ColliderType type
    );

    ColliderTag create_capsule_collider(
        Capsule const & capsule,
        Transform const & transform,
        ColliderType type
    );

    void remove_collider(ColliderTag tag);

    Node & get_node(Scene & scene, NodeID node_id);
    static Transform get_global_transform(Scene & scene, NodeID node_id);

    template<typename Collider>
    void get_overlaps(
        ColliderTag tag,
        Collider const & collider,
        Transform const * transforms,
        Transform const * transforms_history,
        std::unordered_set<Overlap> & overlaps
    ) {
        NodeID node_id = m_node_ids.at(tag);
        Transform const & transform = transforms[node_id];
        Transform const & old_transform = transforms_history[node_id];

        Transform tform;
        tform.position = old_transform.position;
        tform.scale = transform.scale;
        tform.rotation = transform.rotation;

        glm::vec3 pos_diff = transform.position - old_transform.position;

        AABB aabb = calculate_aabb(collider, tform, pos_diff);

        std::array<ColliderType, 2> types =
        {
            ColliderType::collider,
            ColliderType::area,
        };

        for (size_t i = 0; i < types.size(); ++i) {
            ColliderType type = types[i];
            ColliderContainer & container = get_container(type);

            thread_local std::array<std::vector<ColliderID>,
            ColliderShape::total_num_collider_shape> candidates;

            // clear thread_local candidate buffer
            for (auto & candidate : candidates) {
                candidate.resize(0);
            }

            container.aabb_tree.query(tag, collider.get_mask(), aabb, candidates);

            inner_get_overlaps<Collider, MeshCollider>(
                tag,
                collider,
                transforms,
                transforms_history,
                candidates[ColliderShape::mesh],
                m_node_ids,
                container.meshes.map,
                type,
                overlaps
            );

            inner_get_overlaps<Collider, SphereCollider>(
                tag,
                collider,
                transforms,
                transforms_history,
                candidates[ColliderShape::sphere],
                m_node_ids,
                container.spheres.map,
                type,
                overlaps
            );

            inner_get_overlaps<Collider, BoxCollider>(
                tag,
                collider,
                transforms,
                transforms_history,
                candidates[ColliderShape::box],
                m_node_ids,
                container.boxes.map,
                type,
                overlaps
            );

            inner_get_overlaps<Collider, CapsuleCollider>(
                tag,
                collider,
                transforms,
                transforms_history,
                candidates[ColliderShape::capsule],
                m_node_ids,
                container.capsules.map,
                type,
                overlaps
            );
        }
    }

    template<typename Collider, typename Shape>
    CollisionResult move_and_collide(
        Scene & scene,
        ColliderTag tag,
        Collider const & collider,
        glm::vec3 const & movement,
        Transform & transform
    ) {
        assert(tag.type == ColliderType::collider);

        unsigned int iteration = 0;

        CollisionResult ret{};

        glm::vec3 curr_movement = movement;
        while (iteration < MAX_COLLISION_ITER) {
            Collision & collision = ret.collisions[ret.n_collisions];

            AABB aabb = calculate_aabb(collider, transform, curr_movement);

            Transform start_transform = transform;
            transform.position += curr_movement;

            thread_local std::array<std::vector<ColliderID>,
                ColliderShape::total_num_collider_shape> candidates;

            // clear thread_local candidate buffer
            for (auto & candidate : candidates) {
                candidate.resize(0);
            }

            m_colliders.aabb_tree.query(tag, collider.get_mask(), aabb, candidates);
            ColliderTag closest_candidate;
            float t = std::numeric_limits<float>::max();
            closest_candidate.shape = ColliderShape::none;
            GJKRes gjk_res;
            Triangle tri_this;
            Sphere sphere_other;
            Triangle tri_other;
            DiscreteConvexHull<8> box_other;
            Capsule capsule_other;

            inner_collide<Collider, MeshCollider>(
                scene,
                collider,
                start_transform,
                curr_movement,
                candidates[ColliderShape::mesh],
                m_node_ids,
                m_colliders.meshes.map,
                closest_candidate,
                t,
                gjk_res,
                tri_this,
                &tri_other
            );

            inner_collide<Collider, SphereCollider>(
                scene,
                collider,
                start_transform,
                curr_movement,
                candidates[ColliderShape::sphere],
                m_node_ids,
                m_colliders.spheres.map,
                closest_candidate,
                t,
                gjk_res,
                tri_this,
                &sphere_other
            );

            inner_collide<Collider, BoxCollider>(
                scene,
                collider,
                start_transform,
                curr_movement,
                candidates[ColliderShape::box],
                m_node_ids,
                m_colliders.boxes.map,
                closest_candidate,
                t,
                gjk_res,
                tri_this,
                &box_other
            );

            inner_collide<Collider, CapsuleCollider>(
                scene,
                collider,
                start_transform,
                curr_movement,
                candidates[ColliderShape::capsule],
                m_node_ids,
                m_colliders.capsules.map,
                closest_candidate,
                t,
                gjk_res,
                tri_this,
                &capsule_other
            );

            collision.collided =
                closest_candidate.shape != ColliderShape::none;

            switch (closest_candidate.shape) {
                case ColliderShape::mesh: {
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
                case ColliderShape::sphere: {
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
                case ColliderShape::box: {
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
                        box_other
                    );

                    collision.normal = epa_res.normal;
                    collision.impulse = epa_res.normal * epa_res.penetration_depth;

                    break;
                }
                case ColliderShape::capsule: {
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
                        capsule_other
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
                collision.other = closest_candidate;
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
    struct InnerCollide {
        inline static void collide(
            Scene & scene,
            Collider const & collider,
            Transform const & transform,
            glm::vec3 const & curr_movement,
            std::vector<ColliderID> const & ids,
            std::unordered_map<ColliderTag, NodeID> const & node_ids,
            std::unordered_map<ColliderID, Other> const & others,
            ColliderTag & closest_candidate,
            float & t,
            GJKRes & gjk_res,
            Triangle & /*tri_this*/,
            void * data_other
        ) {
            auto shape = collider.get_shape(transform);

            for (ColliderID id : ids) {
                ColliderTag tag_other;
                tag_other.id = id;
                tag_other.shape = Other::shape;
                tag_other.type = ColliderType::collider;

                NodeID other_node_id = node_ids.at(tag_other);
                Transform transform_other = get_global_transform(scene, other_node_id);

                auto shape_other = others.at(id).get_shape(transform_other);

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
                    *reinterpret_cast<decltype(shape_other)*>(data_other) =
                        shape_other;
                }
            }
        }
    };

    template<typename Collider>
    struct InnerCollide<Collider, MeshCollider> {
        inline static void collide(
            Scene & /*scene*/,
            Collider const & collider,
            Transform const & transform,
            glm::vec3 const & curr_movement,
            std::vector<ColliderID> const & ids,
            std::unordered_map<ColliderTag, NodeID> const & /*node_ids*/,
            std::unordered_map<ColliderID, MeshCollider> const & others,
            ColliderTag & closest_candidate,
            float & t,
            GJKRes & gjk_res,
            Triangle & /*tri_this*/,
            void * data_other
        ) {
            auto shape = collider.get_shape(transform);
            auto swept_shape = shape.sweep(curr_movement);
            AABB aabb = swept_shape.aabb();

            thread_local std::vector<Triangle> tris;

            for (ColliderID id : ids) {
                ColliderTag tag_other;
                tag_other.id = id;
                tag_other.shape = ColliderShape::mesh;
                tag_other.type = ColliderType::collider;

                MeshCollider const & mesh_collider = others.at(id);

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
                        *reinterpret_cast<Triangle*>(data_other) = tri;
                    }
                }
            }
        }
    };

    template<typename Other>
    struct InnerCollide<MeshCollider, Other> {
        inline static void collide(
            Scene & scene,
            MeshCollider const & collider,
            Transform const & /*transform*/,
            glm::vec3 const & curr_movement,
            std::vector<ColliderID> const & ids,
            std::unordered_map<ColliderTag, NodeID> const & node_ids,
            std::unordered_map<ColliderID, Other> const & others,
            ColliderTag & closest_candidate,
            float & t,
            GJKRes & gjk_res,
            Triangle & tri_this,
            void * data_other
        ) {
            for (size_t i = 0; i < collider.triangle_cache().size(); i += 3) {
                Triangle triangle;
                triangle.a = collider.triangle_cache()[i];
                triangle.b = collider.triangle_cache()[i+1];
                triangle.c = collider.triangle_cache()[i+2];

                for (ColliderID id : ids) {
                    ColliderTag tag_other;
                    tag_other.id = id;
                    tag_other.shape = Other::shape;
                    tag_other.type = ColliderType::collider;

                    NodeID other_node_id = node_ids.at(tag_other);
                    Transform transform_other = get_global_transform(scene, other_node_id);

                    auto shape_other = others.at(id).get_shape(transform_other);

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
                        *reinterpret_cast<decltype(shape_other)*>(data_other) =
                            shape_other;
                    }
                }
            }
        }
    };

    template<>
    struct InnerCollide<MeshCollider, MeshCollider> {
        inline static void collide(
            Scene & /*scene*/,
            MeshCollider const & collider,
            Transform const & /*transform*/,
            glm::vec3 const & curr_movement,
            std::vector<ColliderID> const & ids,
            std::unordered_map<ColliderTag, NodeID> const & /*node_ids*/,
            std::unordered_map<ColliderID, MeshCollider> const & others,
            ColliderTag & closest_candidate,
            float & t,
            GJKRes & gjk_res,
            Triangle & tri_this,
            void * data_other
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
                    tag_other.shape = ColliderShape::mesh;
                    tag_other.type = ColliderType::collider;

                    MeshCollider const & mesh_collider = others.at(id);

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
                            *reinterpret_cast<Triangle*>(data_other) =
                                tri;
                        }
                    }
                }
            }
        }
    };

    template<typename Collider, typename Other>
    void inner_collide(
        Scene & scene,
        Collider const & collider,
        Transform const & transform,
        glm::vec3 const & curr_movement,
        std::vector<ColliderID> const & ids,
        std::unordered_map<ColliderTag, NodeID> const & node_ids,
        std::unordered_map<ColliderID, Other> const & others,
        ColliderTag & closest_candidate,
        float & t,
        GJKRes & gjk_res,
        Triangle & tri_this,
        void * data_other
    ) {
        InnerCollide<Collider, Other>::collide(
            scene,
            collider,
            transform,
            curr_movement,
            ids,
            node_ids,
            others,
            closest_candidate,
            t,
            gjk_res,
            tri_this,
            data_other
        );
    }

    template<typename Collider, typename Other>
    struct InnerGetOverlaps {
        inline static void get_overlaps(
            ColliderTag tag,
            Collider const & collider,
            Transform const * transforms,
            Transform const * transforms_history,
            std::vector<ColliderID> const & ids,
            std::unordered_map<ColliderTag, NodeID> const & node_ids,
            std::unordered_map<ColliderID, Other> const & others,
            ColliderType other_type,
            std::unordered_set<Overlap> & overlaps
        ) {
            NodeID node_id = node_ids.at(tag);

            Transform tform;
            tform.position = transforms_history[node_id].position;
            tform.scale = transforms[node_id].scale;
            tform.rotation = transforms[node_id].rotation;

            glm::vec3 pos_diff =
                transforms[node_id].position -
                transforms_history[node_id].position;

            auto shape = collider.get_shape(transforms[node_id]).sweep(pos_diff);

            for (ColliderID id : ids) {
                ColliderTag tag_other;
                tag_other.id = id;
                tag_other.shape = Other::shape;
                tag_other.type = other_type;

                NodeID other_node_id = node_ids.at(tag_other);

                Transform tform_other;
                tform_other.position = transforms_history[other_node_id].position;
                tform_other.scale = transforms[other_node_id].scale;
                tform_other.rotation = transforms[other_node_id].rotation;

                glm::vec3 other_pos_diff =
                    transforms[other_node_id].position -
                    transforms_history[other_node_id].position;

                auto shape_other =
                    others.at(id).get_shape(tform_other).sweep(other_pos_diff);

                GJKRes gjk_cand = gjk(shape, shape_other);

                if (gjk_cand.collided) {
                    overlaps.insert({node_id, other_node_id});
                }
            }
        }
    };

    template<typename Collider>
    struct InnerGetOverlaps<Collider, MeshCollider> {
        inline static void get_overlaps(
            ColliderTag tag,
            Collider const & collider,
            Transform const * transforms,
            Transform const * transforms_history,
            std::vector<ColliderID> const & ids,
            std::unordered_map<ColliderTag, NodeID> const & node_ids,
            std::unordered_map<ColliderID, MeshCollider> const & others,
            ColliderType other_type,
            std::unordered_set<Overlap> & overlaps
        ) {
            NodeID node_id = node_ids.at(tag);

            Transform tform;
            tform.position = transforms_history[node_id].position;
            tform.scale = transforms[node_id].scale;
            tform.rotation = transforms[node_id].rotation;

            glm::vec3 pos_diff =
                transforms[node_id].position -
                transforms_history[node_id].position;

            auto shape = collider.get_shape(transforms[node_id]).sweep(pos_diff);

            thread_local std::vector<Triangle> tris;

            for (ColliderID id : ids) {
                ColliderTag tag_other;
                tag_other.id = id;
                tag_other.shape = ColliderShape::mesh;
                tag_other.type = other_type;

                MeshCollider const & mesh_collider = others.at(id);

                NodeID other_node_id = node_ids.at(tag_other);

                // Triangles are currently not swept in overlap, too
                // cumbersome to implement (it would require
                // MeshCollider::collect_triangles() to take into
                // acount swept triangles).
                tris.resize(0);
                mesh_collider.collect_triangles(
                    shape.aabb(),
                    tris
                );
                for (Triangle tri : tris) {
                    GJKRes gjk_cand = gjk(shape, tri);

                    if (gjk_cand.collided) {
                        overlaps.insert({node_id, other_node_id});
                        break;
                    }
                }
            }
        }
    };

    template<typename Other>
    struct InnerGetOverlaps<MeshCollider, Other> {
        inline static void get_overlaps(
            ColliderTag tag,
            MeshCollider const & collider,
            Transform const * transforms,
            Transform const * transforms_history,
            std::vector<ColliderID> const & ids,
            std::unordered_map<ColliderTag, NodeID> const & node_ids,
            std::unordered_map<ColliderID, Other> const & others,
            ColliderType other_type,
            std::unordered_set<Overlap> & overlaps
        ) {
            NodeID node_id = node_ids.at(tag);

            glm::vec3 pos_diff = transforms[node_id].position -
                                 transforms_history[node_id].position;

            for (size_t i = 0; i < collider.triangle_cache().size(); i += 3) {
                Triangle triangle;
                triangle.a = collider.triangle_cache()[i];
                triangle.b = collider.triangle_cache()[i+1];
                triangle.c = collider.triangle_cache()[i+2];

                SweptTriangle swept = triangle.sweep(pos_diff);
                AABB aabb = swept.aabb();

                for (ColliderID id : ids) {
                    ColliderTag tag_other;
                    tag_other.id = id;
                    tag_other.shape = Other::shape;
                    tag_other.type = other_type;

                    NodeID other_node_id = node_ids.at(tag_other);

                    Transform tform_other;
                    tform_other.position = transforms_history[other_node_id].position;
                    tform_other.scale = transforms[other_node_id].scale;
                    tform_other.rotation = transforms[other_node_id].rotation;

                    glm::vec3 other_pos_diff =
                        transforms[other_node_id].position -
                        transforms_history[other_node_id].position;

                    auto shape_other =
                        others.at(id).get_shape(tform_other).sweep(other_pos_diff);

                    if (AABB::intersect(aabb, shape_other.aabb())) {
                        GJKRes gjk_cand = gjk(swept, shape_other);

                        if (gjk_cand.collided) {
                            overlaps.insert({node_id, other_node_id});
                        }
                    }
                }
            }
        }
    };

    template<>
    struct InnerGetOverlaps<MeshCollider, MeshCollider> {
        inline static void get_overlaps(
            ColliderTag tag,
            MeshCollider const & collider,
            Transform const * transforms,
            Transform const * transforms_history,
            std::vector<ColliderID> const & ids,
            std::unordered_map<ColliderTag, NodeID> const & node_ids,
            std::unordered_map<ColliderID, MeshCollider> const & others,
            ColliderType /*other_type*/,
            std::unordered_set<Overlap> & overlaps
        ) {
            NodeID node_id = node_ids.at(tag);

            glm::vec3 pos_diff = transforms[node_id].position -
                                 transforms_history[node_id].position;

            // TODO: Use acceleration structure to avoid O(M*N)
            for (size_t i = 0; i < collider.triangle_cache().size(); i += 3) {
                Triangle triangle;
                triangle.a = collider.triangle_cache()[i];
                triangle.b = collider.triangle_cache()[i+1];
                triangle.c = collider.triangle_cache()[i+2];

                SweptTriangle swept = triangle.sweep(pos_diff);
                AABB aabb = swept.aabb();

                for (ColliderID id : ids) {
                    ColliderTag tag_other;
                    tag_other.id = id;
                    tag_other.shape = ColliderShape::mesh;
                    tag_other.type = ColliderType::collider;

                    MeshCollider const & mesh_collider = others.at(id);
                    NodeID other_node_id = node_ids.at(tag_other);

                    thread_local std::vector<Triangle> tris;
                    tris.resize(0);
                    mesh_collider.collect_triangles(
                        aabb,
                        tris
                    );

                    for (Triangle tri : tris) {
                        if (AABB::intersect(aabb, tri.aabb())) {
                            GJKRes gjk_cand = gjk(swept, tri);

                            if (gjk_cand.collided) {
                                overlaps.insert({node_id, other_node_id});
                                break;
                            }
                        }
                    }
                }
            }
        }
    };

    template<typename Collider, typename Other>
    inline void inner_get_overlaps(
        ColliderTag tag,
        Collider const & collider,
        Transform const * transforms,
        Transform const * transforms_history,
        std::vector<ColliderID> const & ids,
        std::unordered_map<ColliderTag, NodeID> const & node_ids,
        std::unordered_map<ColliderID, Other> const & others,
        ColliderType other_type,
        std::unordered_set<Overlap> & overlaps
    ) {
        InnerGetOverlaps<Collider, Other>::get_overlaps(
            tag,
            collider,
            transforms,
            transforms_history,
            ids,
            node_ids,
            others,
            other_type,
            overlaps
        );
    }

    template<typename Collider>
    inline AABB calculate_aabb(
        Collider const & collider,
        Transform const & transform,
        glm::vec3 const & curr_movement
    ) const {
        return collider.get_shape(transform).sweep(curr_movement).aabb();
    }

    template<>
    inline AABB calculate_aabb<MeshCollider>(
        MeshCollider const & collider,
        Transform const & /*transform*/,
        glm::vec3 const & curr_movement
    ) const {
        return collider.aabb().sweep(curr_movement);
    }

    template<typename Collider, typename Shape>
    inline decltype(std::declval<Shape>().sweep({})) get_swept_shape(
        Transform const & transform,
        glm::vec3 const & curr_movement,
        Collider const & collider,
        Triangle const & /*triangle*/
    ) const {
        return collider.get_shape(transform).sweep(curr_movement);
    }

    template<>
    inline SweptTriangle get_swept_shape<MeshCollider, Triangle>(
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
