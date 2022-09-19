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

    collision_util::CollisionResult move_and_collide(
        NodeID node_id,
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
    collision_util::CollisionResult move_and_collide(
        ColliderTag tag,
        Collider const & collider,
        glm::vec3 const & movement,
        Transform & transform
    ) {
        unsigned int iteration = 0;
        unsigned int max_iter = 5;
        collision_util::CollisionResult ret{};

        glm::vec3 curr_movement = movement;

        while (iteration < max_iter) {
            auto shape = collider.get_shape(transform);
            auto swept_shape = shape.sweep(curr_movement);
            AABB aabb = swept_shape.aabb();
            transform.position += curr_movement;

            collision_util::CollisionResult res{};

            for (auto const & pair : m_sphere_colliders) {
                if (pair.first == tag) continue;
                NodeID other_id = m_node_ids[pair.first];
                Node const & other = get_node(other_id);
                Transform other_transform = other.get_global_transform();

                res = collision_util::gjk(
                    swept_shape,
                    pair.second.get_shape(other_transform)
                );
                if (res.collided) {
                    break;
                }
            }

            if (!res.collided) {
                for (auto const & pair : m_mesh_colliders) {
                    if (pair.first == tag) continue;
                    MeshCollider const & mesh_collider = pair.second;

                    std::vector<Triangle> tris;
                    mesh_collider.collect_triangles(aabb, tris);
                    for (Triangle tri : tris) {
                        res = collision_util::gjk(swept_shape, tri);
                        if (res.collided) {
                            break;
                        }
                    }

                    if (res.collided) {
                        break;
                    }
                }
            }

            if (res.collided) {
                glm::vec3 nudge = res.normal * res.penetration_depth;
                transform.position += nudge;;
                curr_movement = glm::vec3{0.0f};
                ret = res;
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
