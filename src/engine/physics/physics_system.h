#ifndef PRT3_PHYSICS_SYSTEM_H
#define PRT3_PHYSICS_SYSTEM_H

#include "src/engine/physics/gjk.h"

#include "src/engine/physics/aabb_tree.h"
#include "src/engine/physics/collider.h"
#include "src/engine/geometry/shapes.h"
#include "src/engine/rendering/model.h"
#include "src/engine/component/transform.h"
#include "src/engine/scene/node.h"

#include <unordered_map>
#include <cstdint>

namespace prt3 {

class PhysicsSystem {
public:
    Collision move_and_collide(
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

    template<typename Collider>
    Collision move_and_collide(
        Scene & scene,
        ColliderTag tag,
        Collider const & collider,
        glm::vec3 const & movement,
        Transform & transform
    ) {
        unsigned int iteration = 0;
        unsigned int max_iter = 10;

        Collision ret{};

        glm::vec3 curr_movement = movement;
        while (iteration < max_iter) {
            auto shape = collider.get_shape(transform);
            auto swept_shape = shape.sweep(curr_movement);
            AABB aabb = swept_shape.aabb();
            glm::vec3 start_pos = transform.position;
            transform.position += curr_movement;

            static std::array<std::vector<ColliderID>,
                ColliderType::collider_type_none> candidates;
            // clear before use since variable is static
            for (auto & candidate : candidates) {
                candidate.resize(0);
            }
            m_aabb_tree.query(tag, aabb, candidates);

            CollisionResult res{};
            res.t = std::numeric_limits<float>::max();
            ColliderTag res_other;
            Sphere sphere_other;
            Triangle tri_other;

            for (auto const & id :
                 candidates[ColliderType::collider_type_sphere]) {
                ColliderTag other_tag;
                other_tag.id = id;
                other_tag.type = ColliderType::collider_type_sphere;
                NodeID node_id = m_node_ids[other_tag];

                Transform other_transform = get_global_transform(scene, node_id);
                Sphere other_shape =
                    m_sphere_colliders[id].get_shape(other_transform);

                CollisionResult cand =
                    find_collision(
                        shape,
                        other_shape,
                        curr_movement
                    );
                if (cand.collided && cand.t < res.t) {
                    res = cand;
                    res_other = other_tag;
                    sphere_other = other_shape;
                }
            }

            if (!res.collided) {
                for (auto const & id :
                     candidates[ColliderType::collider_type_mesh]) {
                    MeshCollider const & mesh_collider = m_mesh_colliders[id];

                    static std::vector<Triangle> tris;
                    tris.resize(0);
                    mesh_collider.collect_triangles(
                        aabb,
                        tris
                    );
                    for (Triangle tri : tris) {
                        CollisionResult cand =
                            find_collision(
                                shape,
                                tri,
                                curr_movement
                            );
                        if (cand.collided && cand.t < res.t) {
                            res = cand;
                            res_other.id = id;
                            res_other.type = ColliderType::collider_type_mesh;
                            tri_other = tri;
                        }
                    }
                }
            }

            if (res.collided) {
                EPARes epa_res;
                auto swept = shape.sweep(curr_movement * res.t);
                switch (res_other.type) {
                    case ColliderType::collider_type_sphere: {
                        epa_res = epa(
                            res.simplex,
                            res.n_simplex,
                            swept,
                            sphere_other
                        );
                    }
                    case ColliderType::collider_type_mesh: {
                        epa_res = epa(
                            res.simplex,
                            res.n_simplex,
                            swept,
                            tri_other
                        );
                    }
                    default: {}
                }

                glm::vec3 nudge = epa_res.normal * epa_res.penetration_depth;

                transform.position = start_pos + res.t * curr_movement + nudge;

                glm::vec3 remain = (1.0f - res.t) * curr_movement;

                curr_movement = remain - glm::dot(remain, epa_res.normal) * epa_res.normal;

                ret.collided = true;
                ret.normal = epa_res.normal;

                if (glm::dot(epa_res.normal, glm::vec3{0.0f, 1.0f, 0.0f}) > 0.707) {
                    ret.grounded = true;
                    ret.ground_normal = epa_res.normal;
                }
            } else {
                break;
            }

            ++iteration;
        }
        return ret;
    }

    friend class Scene;
    friend class ColliderComponent;
};

} // namespace prt3

#endif
