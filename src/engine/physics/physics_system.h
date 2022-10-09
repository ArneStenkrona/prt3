#ifndef PRT3_PHYSICS_SYSTEM_H
#define PRT3_PHYSICS_SYSTEM_H

#include "src/engine/physics/gjk.h"

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
    PhysicsSystem(Scene & scene);

    Collision move_and_collide(NodeID node_id,
                               glm::vec3 const & movement);

    void add_mesh_collider(NodeID node_id,
                           Model const & model);

    void add_sphere_collider(NodeID node_id,
                             Sphere const & sphere);

private:
    std::unordered_map<NodeID, ColliderTag> m_tags;
    std::unordered_map<ColliderTag, NodeID> m_node_ids;

    std::unordered_map<ColliderTag, MeshCollider> m_mesh_colliders;
    ColliderID m_next_mesh_id;
    std::unordered_map<ColliderTag, SphereCollider> m_sphere_colliders;
    ColliderID m_next_sphere_id;

    Scene & m_scene;

    ColliderTag create_collider_from_model(Model const & model);

    ColliderTag create_sphere_collider(Sphere const & sphere);

    Node & get_node(NodeID node_id);

    template<typename Collider>
    Collision move_and_collide(
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

            collision_util::CollisionResult res{};
            res.t = std::numeric_limits<float>::max();
            ColliderTag res_other;
            Sphere sphere_other;
            Triangle tri_other;

            for (auto const & pair : m_sphere_colliders) {
                if (pair.first == tag) continue;
                NodeID other_id = m_node_ids[pair.first];
                Node const & other = get_node(other_id);
                Transform other_transform = other.get_global_transform();

                collision_util::CollisionResult cand =
                    collision_util::find_collision(
                        shape,
                        pair.second.get_shape(other_transform),
                        curr_movement
                    );
                if (cand.collided && cand.t < res.t) {
                    res = cand;
                    res_other = pair.first;
                    sphere_other = pair.second.get_shape(other_transform);
                }
            }

            if (!res.collided) {
                for (auto const & pair : m_mesh_colliders) {
                    if (pair.first == tag) continue;
                    MeshCollider const & mesh_collider = pair.second;

                    std::vector<Triangle> tris;
                    mesh_collider.collect_triangles(
                                    aabb,
                                    tris);
                    for (Triangle tri : tris) {
                        collision_util::CollisionResult cand =
                            collision_util::find_collision(
                                shape,
                                tri,
                                curr_movement
                            );
                        if (cand.collided && cand.t < res.t) {
                            res = cand;
                            res_other = pair.first;
                            tri_other = tri;
                        }
                    }
                }
            }

            if (res.collided) {
                collision_util::EPARes epa_res;
                auto swept = shape.sweep(curr_movement * res.t);
                switch (res_other.type) {
                    case ColliderType::collider_type_sphere: {
                        epa_res = collision_util::epa(
                            res.simplex,
                            res.n_simplex,
                            swept,
                            sphere_other
                        );
                    }
                    case ColliderType::collider_type_mesh: {
                        epa_res = collision_util::epa(
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
};

} // namespace prt3

#endif
