#include "physics_system.h"

#include "src/engine/scene/scene.h"
#include "src/util/geometry_util.h"
#include "src/util/mesh_util.h"

using namespace prt3;

ColliderTag PhysicsSystem::add_mesh_collider(
    NodeID node_id,
    ColliderType type,
    std::vector<glm::vec3> && triangles,
    Transform const & transform
) {
    ColliderTag tag = create_collider_from_triangles(
        std::move(triangles),
        transform,
        type
    );

    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;

    return tag;
}

ColliderTag PhysicsSystem::add_mesh_collider(
    NodeID node_id,
    ColliderType type,
    std::vector<glm::vec3> const & triangles,
    Transform const & transform
) {
    ColliderTag tag = create_collider_from_triangles(
        triangles,
        transform,
        type
    );

    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;

    return tag;
}

ColliderTag PhysicsSystem::add_mesh_collider(
    NodeID node_id,
    ColliderType type,
    Model const & model,
    Transform const & transform
) {
    ColliderTag tag = create_collider_from_model(
        model,
        transform,
        type
    );

    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;

    return tag;
}

ColliderTag PhysicsSystem::add_sphere_collider(
    NodeID node_id,
    ColliderType type,
    Sphere const & sphere,
    Transform const & transform
) {
    ColliderTag tag = create_sphere_collider(
        sphere,
        transform,
        type
    );
    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;

    return tag;
}

ColliderTag PhysicsSystem::add_capsule_collider(
    NodeID node_id,
    ColliderType type,
    Capsule const & capsule,
    Transform const & transform
) {
    ColliderTag tag = create_capsule_collider(
        capsule,
        transform,
        type
    );
    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;

    return tag;
}

ColliderTag PhysicsSystem::add_box_collider(
    NodeID node_id,
    ColliderType type,
    glm::vec3 const & dimensions,
    glm::vec3 const & center,
    Transform const & transform
) {
    ColliderTag tag = create_box_collider(
        dimensions,
        center,
        transform,
        type
    );
    m_tags[node_id] = tag;
    m_node_ids[tag] = node_id;

    return tag;
}

void PhysicsSystem::remove_collider(ColliderTag tag) {
    ColliderContainer & container = get_container(tag.type);
    switch (tag.shape) {
        case ColliderShape::mesh: {
            container.meshes.map.erase(tag.id);
            break;
        }
        case ColliderShape::sphere: {
            container.spheres.map.erase(tag.id);
            break;
        }
        case ColliderShape::box: {
            container.boxes.map.erase(tag.id);
            break;
        }
        case ColliderShape::capsule: {
            container.capsules.map.erase(tag.id);
            break;
        }
        default: {
            return;
        }
    }

    NodeID node_id = m_node_ids[tag];
    m_node_ids.erase(tag);
    m_tags.erase(node_id);

    container.aabb_tree.remove(tag);
}

CollisionResult PhysicsSystem::move_and_collide(
    Scene & scene,
    NodeID node_id,
    glm::vec3 const & movement
) {
    CollisionResult res{};

    ColliderTag const & tag = m_tags[node_id];
    Node & node = scene.get_node(node_id);

    Transform transform = node.get_global_transform(scene);
    Transform start_transform = transform;

    switch (tag.shape) {
        case ColliderShape::mesh: {
            MeshCollider const & col = m_colliders.meshes.map[tag.id];
            res = move_and_collide<MeshCollider, Triangle>(
                scene, tag, col, movement, transform
            );
            break;
        }
        case ColliderShape::sphere: {
            SphereCollider const & col = m_colliders.spheres.map[tag.id];
            res = move_and_collide<SphereCollider, Sphere>(
                scene, tag, col, movement, transform
            );
            break;
        }
        case ColliderShape::box: {
            BoxCollider const & col = m_colliders.boxes.map[tag.id];
            res = move_and_collide<BoxCollider, DiscreteConvexHull<8> >(
                scene, tag, col, movement, transform
            );
            break;
        }
        case ColliderShape::capsule: {
            CapsuleCollider const & col = m_colliders.capsules.map[tag.id];
            res = move_and_collide<CapsuleCollider, Capsule>(
                scene, tag, col, movement, transform
            );
            break;
        }
        default: {}
    }
    node.set_global_transform(scene, transform);

    if (transform != start_transform) {
        AABB aabb{};
        CollisionLayer layer{};
        switch (tag.shape) {
            case ColliderShape::mesh: {
                m_colliders.meshes.map[tag.id].set_transform(transform);
                aabb = m_colliders.meshes.map[tag.id].aabb();
                layer = m_colliders.meshes.map[tag.id].get_layer();
                break;
            }
            case ColliderShape::sphere: {
                aabb = m_colliders.spheres.map[tag.id].get_shape(transform).aabb();
                layer = m_colliders.spheres.map[tag.id].get_layer();
                break;
            }
            case ColliderShape::box: {
                aabb = m_colliders.boxes.map[tag.id].get_shape(transform).aabb();
                layer = m_colliders.boxes.map[tag.id].get_layer();
                break;
            }
            case ColliderShape::capsule: {
                aabb = m_colliders.capsules.map[tag.id].get_shape(transform).aabb();
                layer = m_colliders.capsules.map[tag.id].get_layer();
                break;
            }
            default: { assert(false && "Invalid collision shape"); return res; }
        }
        m_colliders.aabb_tree.update({tag, layer, aabb});
    }
    return res;
}

bool PhysicsSystem::raycast(
    glm::vec3 origin,
    glm::vec3 direction,
    float max_distance,
    CollisionLayer mask,
    ColliderTag ignore,
    RayHit & hit
) const {
    thread_local std::array<std::vector<ColliderID>,
            ColliderShape::total_num_collider_shape> candidates;

    m_colliders.aabb_tree.query_raycast(
        origin,
        direction,
        max_distance,
        mask,
        candidates
    );

    glm::vec3 dest = origin + direction * max_distance;

    AABB aabb;
    aabb.lower_bound = glm::min(origin, dest);
    aabb.upper_bound = glm::max(origin, dest);

    thread_local std::vector<Triangle> triangles;
    float min_t = std::numeric_limits<float>::infinity();

    for (ColliderID id : candidates[ColliderShape::mesh]) {
        MeshCollider const & col =
            get_mesh_collider(id, ColliderType::collider);

        ColliderTag tag;
        tag.id = id;
        tag.shape = ColliderShape::mesh;
        tag.type = ColliderType::collider;

        if (tag == ignore) continue;

        triangles.clear();
        col.collect_triangles(aabb, triangles);

        for (Triangle const & tri : triangles) {
            glm::vec3 intersection;
            if (triangle_ray_intersect(
                origin,
                direction * max_distance,
                tri.a,
                tri.b,
                tri.c,
                intersection
            )) {
                float t = glm::distance(origin, intersection);
                if (t < min_t) {
                    hit.position = intersection;
                    hit.tag = tag;
                    hit.node_id = m_node_ids.at(tag);
                    min_t = t;
                }
            }
        }
    }

    // TODO: implement other collider shapes

    return min_t != std::numeric_limits<float>::infinity();
}

ColliderTag PhysicsSystem::create_collider_from_triangles(
    std::vector<glm::vec3> && triangles,
    Transform const & transform,
    ColliderType type
) {
    ColliderContainer & container = get_container(type);

    ColliderTag tag;
    tag.shape = ColliderShape::mesh;
    tag.id = container.meshes.next_id;
    tag.type = type;
    ++container.meshes.next_id;

    MeshCollider & col = container.meshes.map[tag.id];
    col.set_transform(transform);
    col.set_triangles(std::move(triangles));
    col.set_layer(1 << 0);
    col.set_mask(1 << 0);

    container.aabb_tree.insert(tag, col.get_layer(), col.aabb());

    return tag;
}

ColliderTag PhysicsSystem::create_collider_from_triangles(
    std::vector<glm::vec3> const & triangles,
    Transform const & transform,
    ColliderType type
) {
    ColliderContainer & container = get_container(type);

    ColliderTag tag;
    tag.shape = ColliderShape::mesh;
    tag.id = container.meshes.next_id;
    tag.type = type;
    ++container.meshes.next_id;

    MeshCollider & col = container.meshes.map[tag.id];
    col.set_transform(transform);
    col.set_triangles(triangles);
    col.set_layer(1 << 0);
    col.set_mask(1 << 0);

    container.aabb_tree.insert(tag, col.get_layer(), col.aabb());

    return tag;
}

ColliderTag PhysicsSystem::create_collider_from_model(
    Model const & model,
    Transform const & transform,
    ColliderType type
) {
    ColliderContainer & container = get_container(type);

    std::vector<Model::Vertex> v_buf = model.vertex_buffer();
    std::vector<uint32_t> i_buf = model.index_buffer();

    std::vector<glm::vec3> tris;
    tris.resize(i_buf.size());

    for (auto const & node : model.nodes()) {
        if (node.mesh_index == -1) {
            continue;
        }

        glm::mat4 tform = node.inherited_transform.to_matrix();

        auto const & mesh = model.meshes()[node.mesh_index];

        auto end_index = mesh.start_index + mesh.num_indices;
        for (auto i = mesh.start_index; i < end_index; ++i) {
            tris[i] = glm::vec3(tform * glm::vec4(v_buf[i_buf[i]].position, 1.0f));
        }
    }

    ColliderTag tag;
    tag.shape = ColliderShape::mesh;
    tag.id = container.meshes.next_id;
    tag.type = type;
    ++container.meshes.next_id;

    MeshCollider & col = container.meshes.map[tag.id];
    col.set_transform(transform);
    col.set_triangles(std::move(tris));
    col.set_layer(1 << 0);
    col.set_mask(1 << 0);

    container.aabb_tree.insert(tag, col.get_layer(), col.aabb());

    return tag;
}

ColliderTag PhysicsSystem::create_sphere_collider(
    Sphere const & sphere,
    Transform const & transform,
    ColliderType type
) {
    ColliderContainer & container = get_container(type);

    ColliderTag tag;
    tag.shape = ColliderShape::sphere;
    tag.id = container.spheres.next_id;
    tag.type = type;
    ++container.spheres.next_id;

    container.spheres.map[tag.id] = SphereCollider{sphere};
    SphereCollider & col = container.spheres.map.at(tag.id);
    col.set_layer(1 << 0);
    col.set_mask(1 << 0);

    container.aabb_tree.insert(
        tag,
        col.get_layer(),
        col.get_shape(transform).aabb()
    );

    return tag;
}

ColliderTag PhysicsSystem::create_box_collider(
    glm::vec3 dimensions,
    glm::vec3 center,
    Transform const & transform,
    ColliderType type
) {
    ColliderContainer & container = get_container(type);

    ColliderTag tag;
    tag.shape = ColliderShape::box;
    tag.id = container.boxes.next_id;
    tag.type = type;
    ++container.boxes.next_id;

    container.boxes.map[tag.id] = BoxCollider{dimensions, center};
    BoxCollider & col = container.boxes.map.at(tag.id);
    col.set_layer(1 << 0);
    col.set_mask(1 << 0);

    container.aabb_tree.insert(
        tag,
        col.get_layer(),
        col.get_shape(transform).aabb()
    );

    return tag;
}

ColliderTag PhysicsSystem::create_capsule_collider(
    Capsule const & capsule,
    Transform const & transform,
    ColliderType type
) {
    ColliderContainer & container = get_container(type);

    ColliderTag tag;
    tag.shape = ColliderShape::capsule;
    tag.id = container.capsules.next_id;
    tag.type = type;
    ++container.capsules.next_id;

    container.capsules.map[tag.id] = CapsuleCollider{capsule};
    CapsuleCollider & col = container.capsules.map.at(tag.id);
    col.set_layer(1 << 0);
    col.set_mask(1 << 0);

    container.aabb_tree.insert(
        tag,
        col.get_layer(),
        col.get_shape(transform).aabb()
    );

    return tag;
}


Node & PhysicsSystem::get_node(Scene & scene, NodeID node_id) {
    return scene.get_node(node_id);
}

Transform PhysicsSystem::get_global_transform(
    Scene const & scene,
    NodeID node_id) {
    return scene.get_node(node_id).get_global_transform(scene);
}

void PhysicsSystem::clear() {
    m_tags.clear();
    m_node_ids.clear();

    m_colliders.meshes.map.clear();
    m_colliders.meshes.next_id = 0;
    m_colliders.spheres.map.clear();
    m_colliders.spheres.next_id = 0;
    m_colliders.boxes.map.clear();
    m_colliders.boxes.next_id = 0;
    m_colliders.capsules.map.clear();
    m_colliders.capsules.next_id = 0;

    m_colliders.aabb_tree.clear();

    m_areas.meshes.map.clear();
    m_areas.meshes.next_id = 0;
    m_areas.spheres.map.clear();
    m_areas.spheres.next_id = 0;
    m_areas.boxes.map.clear();
    m_areas.boxes.next_id = 0;
    m_areas.capsules.map.clear();
    m_areas.capsules.next_id = 0;

    m_areas.aabb_tree.clear();
}

void PhysicsSystem::update(
    Transform const * transforms,
    Transform const * transforms_history
) {
    std::array<ColliderContainer *, 2> containers =
    {
        &m_colliders,
        &m_areas
    };

    std::array<ColliderType, 2> types =
    {
        ColliderType::collider,
        ColliderType::area,
    };

    for (unsigned int i = 0; i < containers.size(); ++i) {
        thread_local std::vector<DynamicAABBTree::UpdatePackage> packages;
        packages.resize(0);

        ColliderContainer & container = *containers[i];
        ColliderType type = types[i];

        for (auto & pair : container.meshes.map) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.shape = ColliderShape::mesh;
            tag.type = type;
            NodeID id = m_node_ids.at(tag);

            MeshCollider & col = pair.second;

            Transform const & t_curr = transforms[id];
            Transform const & t_hist = transforms_history[id];

            bool stale = col.m_layer_changed || t_curr != t_hist;

            if (stale) {
                col.set_transform(t_curr);
                packages.push_back({
                    tag,
                    col.get_layer(),
                    col.aabb()
                });
            }
        }

        for (auto & pair : container.spheres.map) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.shape = ColliderShape::sphere;
            tag.type = type;
            NodeID id = m_node_ids.at(tag);

            SphereCollider & col = pair.second;

            Transform const & t_curr = transforms[id];
            Transform const & t_hist = transforms_history[id];

            bool stale = col.m_layer_changed || t_curr != t_hist;

            if (stale) {
                packages.push_back({
                    tag,
                    col.get_layer(),
                    col.get_shape(t_curr).aabb()
                });
            }
        }

        for (auto & pair : container.boxes.map) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.shape = ColliderShape::box;
            tag.type = type;
            NodeID id = m_node_ids.at(tag);

            BoxCollider & col = pair.second;

            Transform const & t_curr = transforms[id];
            Transform const & t_hist = transforms_history[id];

            bool stale = col.m_layer_changed || t_curr != t_hist;

            if (stale) {
                packages.push_back({
                    tag,
                    col.get_layer(),
                    col.get_shape(t_curr).aabb()
                });
            }
        }

        for (auto & pair : container.capsules.map) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.shape = ColliderShape::capsule;
            tag.type = type;
            NodeID id = m_node_ids.at(tag);

            CapsuleCollider & col = pair.second;

            Transform const & t_curr = transforms[id];
            Transform const & t_hist = transforms_history[id];

            bool stale = col.m_layer_changed || t_curr != t_hist;

            if (stale) {
                packages.push_back({
                    tag,
                    col.get_layer(),
                    col.get_shape(t_curr).aabb()
                });
            }
        }

        container.aabb_tree.update(
            packages.data(),
            packages.size()
        );
    }

    update_areas(transforms, transforms_history);
}

void PhysicsSystem::update_areas(
    Transform const * transforms,
    Transform const * transforms_history
) {
    thread_local std::unordered_set<Overlap> overlaps;
    overlaps.clear();

    for (auto & pair : m_areas.meshes.map) {
        ColliderTag tag;
        tag.id = pair.first;
        tag.shape = ColliderShape::mesh;
        tag.type = ColliderType::area;
        get_overlaps(
            tag,
            pair.second,
            transforms,
            transforms_history,
            overlaps
        );
    }

    for (auto & pair : m_areas.spheres.map) {
        ColliderTag tag;
        tag.id = pair.first;
        tag.shape = ColliderShape::sphere;
        tag.type = ColliderType::area;
        get_overlaps(
            tag,
            pair.second,
            transforms,
            transforms_history,
            overlaps
        );
    }

    for (auto & pair : m_areas.boxes.map) {
        ColliderTag tag;
        tag.id = pair.first;
        tag.shape = ColliderShape::box;
        tag.type = ColliderType::area;
        get_overlaps(
            tag,
            pair.second,
            transforms,
            transforms_history,
            overlaps
        );
    }

    for (auto & pair : m_areas.capsules.map) {
        ColliderTag tag;
        tag.id = pair.first;
        tag.shape = ColliderShape::capsule;
        tag.type = ColliderType::area;
        get_overlaps(
            tag,
            pair.second,
            transforms,
            transforms_history,
            overlaps
        );
    }

    m_node_to_overlaps.clear();
    for (std::vector<NodeID> & overlaps : m_overlaps) {
        overlaps.resize(0);
    }

    unsigned int next_ind = 0;
    for (Overlap const & overlap : overlaps) {
        if (m_node_to_overlaps.find(overlap.a()) == m_node_to_overlaps.end()) {
            m_node_to_overlaps[overlap.a()] = next_ind;
            ++next_ind;
        }
        if (m_node_to_overlaps.find(overlap.b()) == m_node_to_overlaps.end()) {
            m_node_to_overlaps[overlap.b()] = next_ind;
            ++next_ind;
        }
    }

    if (next_ind > m_overlaps.size()) {
        m_overlaps.resize(next_ind);
    }

    for (Overlap const & overlap : overlaps) {
        m_overlaps[m_node_to_overlaps.at(overlap.a())].push_back(overlap.b());
        m_overlaps[m_node_to_overlaps.at(overlap.b())].push_back(overlap.a());
    }
}

std::vector<NodeID> const & PhysicsSystem::get_overlaps(NodeID node_id) const {
    if (m_node_to_overlaps.find(node_id) == m_node_to_overlaps.end()) {
        thread_local std::vector<NodeID> empty;
        return empty;
    }
    return m_overlaps[m_node_to_overlaps.at(node_id)];
}

void PhysicsSystem::update_mesh_data(
    Renderer & renderer,
    ColliderType type
) {
    ColliderContainer const & container = get_container(type);

    for (auto const & pair : container.meshes.map) {
        auto const & collider = pair.second;
        if (collider.m_changed) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.shape = ColliderShape::mesh;
            tag.type = type;

            auto const & tris = collider.triangles();
            size_t n = tris.size();

            thread_local std::vector<glm::vec3> lines;
            lines.resize(n * 2);

            size_t n_tri = n / 3;
            size_t li = 0;
            for (size_t i = 0; i < n_tri; ++i) {
                lines[li] = tris[3*i];
                ++li;
                lines[li] = tris[3*i+1];
                ++li;
                lines[li] = tris[3*i+1];
                ++li;
                lines[li] = tris[3*i+2];
                ++li;
                lines[li] = tris[3*i+2];
                ++li;
                lines[li] = tris[3*i];
                ++li;
            }

            if (m_collider_meshes.find(tag) == m_collider_meshes.end()) {
                m_collider_meshes[tag] =
                    renderer.upload_pos_mesh(lines.data(), lines.size());
            } else {
                renderer.update_pos_mesh(
                    m_collider_meshes.at(tag), lines.data(), lines.size()
                );
            }

            collider.m_changed = false;
        }
    }
}

void PhysicsSystem::update_sphere_data(
    Renderer & renderer,
    ColliderType type
) {
    ColliderContainer const & container = get_container(type);

    for (auto const & pair : container.spheres.map) {
        auto const & collider = pair.second;
        if (collider.m_changed) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.shape = ColliderShape::sphere;
            tag.type = type;

            // It might be easer to just have one shape for all spheres
            // and modify the model matrix, but when capsule colliders
            // are implemented it makes sense to create unique meshes
            // for all
            thread_local std::vector<glm::vec3> lines;
            unsigned int n = 64;
            lines.resize(n);
            float d_theta = (2.0f * glm::pi<float>()) / n;

            glm::vec3 const & pos = collider.base_shape().position;
            float r = collider.base_shape().radius;

            for (unsigned int i = 0; i < n; ++i) {
                float theta_a = d_theta * i;
                float theta_b = theta_a + d_theta;
                glm::vec3 a = glm::vec3{r * cos(theta_a), 0.0f, r * sin(theta_a)} + pos;
                glm::vec3 b = glm::vec3{r * cos(theta_b), 0.0f, r * sin(theta_b)} + pos;

                lines.push_back(a);
                lines.push_back(b);
            }

            for (unsigned int i = 0; i < n; ++i) {
                float theta_a = d_theta * i;
                float theta_b = theta_a + d_theta;
                glm::vec3 a = glm::vec3{r * cos(theta_a), r * sin(theta_a), 0.0f} + pos;
                glm::vec3 b = glm::vec3{r * cos(theta_b), r * sin(theta_b), 0.0f} + pos;

                lines.push_back(a);
                lines.push_back(b);
            }

            for (unsigned int i = 0; i < n; ++i) {
                float theta_a = d_theta * i;
                float theta_b = theta_a + d_theta;
                glm::vec3 a = glm::vec3{0.0f, r * cos(theta_a), r * sin(theta_a)} + pos;
                glm::vec3 b = glm::vec3{0.0f, r * cos(theta_b), r * sin(theta_b)} + pos;

                lines.push_back(a);
                lines.push_back(b);
            }

            if (m_collider_meshes.find(tag) == m_collider_meshes.end()) {
                m_collider_meshes[tag] =
                    renderer.upload_pos_mesh(lines.data(), lines.size());
            } else {
                renderer.update_pos_mesh(
                    m_collider_meshes.at(tag), lines.data(), lines.size()
                );
            }

            collider.m_changed = false;
        }
    }
}

void PhysicsSystem::update_box_data(
    Renderer & renderer,
    ColliderType type
) {
    ColliderContainer const & container = get_container(type);

    for (auto const & pair : container.boxes.map) {
        auto const & collider = pair.second;
        if (collider.m_changed) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.shape = ColliderShape::box;
            tag.type = type;

            std::array<glm::vec3, 24> lines;
            glm::vec3 max = collider.center() + 0.5f * collider.dimensions();
            glm::vec3 min = collider.center() - 0.5f * collider.dimensions();

            insert_line_box(min, max, lines.data());

            if (m_collider_meshes.find(tag) == m_collider_meshes.end()) {
                m_collider_meshes[tag] =
                    renderer.upload_pos_mesh(lines.data(), lines.size());
            } else {
                renderer.update_pos_mesh(
                    m_collider_meshes.at(tag), lines.data(), lines.size()
                );
            }

            collider.m_changed = false;
        }
    }
}

void PhysicsSystem::update_capsule_data(
    Renderer & renderer,
    ColliderType type
) {
    ColliderContainer const & container = get_container(type);

    for (auto const & pair : container.capsules.map) {
        auto const & collider = pair.second;
        if (collider.m_changed) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.shape = ColliderShape::capsule;
            tag.type = type;

            // It might be easer to just have one shape for all spheres
            // and modify the model matrix, but when capsule colliders
            // are implemented it makes sense to create unique meshes
            // for all
            thread_local std::vector<glm::vec3> lines;
            unsigned int n = 64;
            lines.resize(n);
            float d_theta = (2.0f * glm::pi<float>()) / n;
            float pi = glm::pi<float>();
            float half_pi = glm::half_pi<float>();

            glm::vec3 const & start = collider.base_shape().start;
            glm::vec3 const & end = collider.base_shape().end;
            float r = collider.base_shape().radius;

            float l = glm::length(end - start);

            glm::vec3 up = glm::vec3{0.0f, 1.0f, 0.0f};
            glm::vec3 e0 = l == 0.0f ? up : glm::normalize(end - start);
            glm::vec3 temp = glm::length(glm::cross(e0, up)) > 0.01f ?
                up : glm::vec3{1.0f, 0.0f, 0.0f};
            glm::vec3 e1 = glm::cross(e0, temp);
            glm::vec3 e2 = glm::cross(e0, e1);

            glm::mat4 tform = glm::lookAt(
                glm::vec3{0.0f},
                e2,
                e0
            );

            for (unsigned int i = 0; i < n; ++i) {
                float theta_a = d_theta * i;
                float theta_b = theta_a + d_theta;
                glm::vec3 a = tform * glm::vec4{r * cos(theta_a), 0.0f, r * sin(theta_a), 1.0f};
                glm::vec3 b = tform * glm::vec4{r * cos(theta_b), 0.0f, r * sin(theta_b), 1.0f};
                a += start;
                b += start;

                lines.push_back(a);
                lines.push_back(b);
            }

            for (unsigned int i = 0; i < n; ++i) {
                float theta_a = d_theta * i;
                float theta_b = theta_a + d_theta;
                glm::vec3 a = tform * glm::vec4{r * cos(theta_a), l, r * sin(theta_a), 1.0f};
                glm::vec3 b = tform * glm::vec4{r * cos(theta_b), l, r * sin(theta_b), 1.0f};
                a += start;
                b += start;

                lines.push_back(a);
                lines.push_back(b);
            }

            for (unsigned int i = 0; i < n / 2; ++i) {
                float theta_a = pi + d_theta * i;
                float theta_b = theta_a + d_theta;
                glm::vec3 a = tform * glm::vec4{r * cos(theta_a), r * sin(theta_a), 0.0f, 1.0f};
                glm::vec3 b = tform * glm::vec4{r * cos(theta_b), r * sin(theta_b), 0.0f, 1.0f};
                a += start;
                b += start;

                lines.push_back(a);
                lines.push_back(b);
            }

            for (unsigned int i = 0; i < n / 2; ++i) {
                float theta_a = d_theta * i;
                float theta_b = theta_a + d_theta;
                glm::vec3 a = tform * glm::vec4{r * cos(theta_a), r * sin(theta_a) + l, 0.0f, 1.0f};
                glm::vec3 b = tform * glm::vec4{r * cos(theta_b), r * sin(theta_b) + l, 0.0f, 1.0f};
                a += start;
                b += start;

                lines.push_back(a);
                lines.push_back(b);
            }

            for (unsigned int i = 0; i < n / 2; ++i) {
                float theta_a = half_pi + d_theta * i;
                float theta_b = theta_a + d_theta;
                glm::vec3 a = tform * glm::vec4{0.0f, r * cos(theta_a), r * sin(theta_a), 1.0f};
                glm::vec3 b = tform * glm::vec4{0.0f, r * cos(theta_b), r * sin(theta_b), 1.0f};
                a += start;
                b += start;

                lines.push_back(a);
                lines.push_back(b);
            }

            for (unsigned int i = 0; i < n / 2; ++i) {
                float theta_a = -half_pi + d_theta * i;
                float theta_b = theta_a + d_theta;
                glm::vec3 a = tform * glm::vec4{0.0f, r * cos(theta_a) + l, r * sin(theta_a), 1.0f};
                glm::vec3 b = tform * glm::vec4{0.0f, r * cos(theta_b) + l, r * sin(theta_b), 1.0f};
                a += start;
                b += start;

                lines.push_back(a);
                lines.push_back(b);
            }

            {
                glm::vec3 a = tform * glm::vec4{r, 0.0f, 0.0f, 1.0f};
                glm::vec3 b = tform * glm::vec4{r, l, 0.0f, 1.0f};
                a += start;
                b += start;

                lines.push_back(a);
                lines.push_back(b);
            }

            {
                glm::vec3 a = tform * glm::vec4{-r, 0.0f, 0.0f, 1.0f};
                glm::vec3 b = tform * glm::vec4{-r, l, 0.0f, 1.0f};
                a += start;
                b += start;

                lines.push_back(a);
                lines.push_back(b);
            }

            {
                glm::vec3 a = tform * glm::vec4{0.0f, 0.0f, r, 1.0f};
                glm::vec3 b = tform * glm::vec4{0.0f, l, r, 1.0f};
                a += start;
                b += start;

                lines.push_back(a);
                lines.push_back(b);
            }

            {
                glm::vec3 a = tform * glm::vec4{0.0f, 0.0f, -r, 1.0f};
                glm::vec3 b = tform * glm::vec4{0.0f, l, -r, 1.0f};
                a += start;
                b += start;

                lines.push_back(a);
                lines.push_back(b);
            }

            if (m_collider_meshes.find(tag) == m_collider_meshes.end()) {
                m_collider_meshes[tag] =
                    renderer.upload_pos_mesh(lines.data(), lines.size());
            } else {
                renderer.update_pos_mesh(
                    m_collider_meshes.at(tag), lines.data(), lines.size()
                );
            }

            collider.m_changed = false;
        }
    }
}

void PhysicsSystem::collect_render_data(
    Renderer & renderer,
    Transform const * transforms,
    NodeID selected,
    EditorRenderData & data
) {
    std::array<ColliderType, 2> types =
    {
        ColliderType::collider,
        ColliderType::area,
    };

    for (ColliderType type : types) {
        update_mesh_data(renderer, type);
        update_sphere_data(renderer, type);
        update_box_data(renderer, type);
        update_capsule_data(renderer, type);
    }

    if (selected == NO_NODE) {
        size_t i = data.line_data.size();
        data.line_data.resize(i + m_tags.size());
        for (auto const & pair : m_tags) {
            data.line_data[i].mesh_id = m_collider_meshes.at(pair.second);
            // NOTE: this is incorrect for spheres/capsules, since the collider
            //       transforms the radius according to
            //       compMax(transform.scale) * radius. Sphere radii
            //       are one dimensional, hence this limitation.
            // TODO: resolve this discrepancy
            data.line_data[i].transform =
                transforms[pair.first].to_matrix();

            data.line_data[i].color = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
            ++i;
        }
    } else if (m_tags.find(selected) != m_tags.end()) {
        size_t index = data.line_data.size();
        data.line_data.resize(index + 1);
        data.line_data[index].mesh_id =
            m_collider_meshes.at(m_tags.at(selected));
        data.line_data[index].transform = transforms[selected].to_matrix();
        data.line_data[index].color = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
    }
}
