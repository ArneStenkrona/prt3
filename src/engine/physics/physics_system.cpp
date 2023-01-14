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
    std::vector<glm::vec3> const & triangles,
    Transform const & transform
) {
    ColliderTag tag = create_collider_from_triangles(
        triangles,
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

ColliderTag PhysicsSystem::add_box_collider(
    NodeID node_id,
    glm::vec3 const & dimensions,
    glm::vec3 const & center,
    Transform const & transform
) {
    ColliderTag tag = create_box_collider(
        dimensions,
        center,
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
        case ColliderType::collider_type_box: {
            m_box_colliders.erase(tag.id);
            break;
        }
        default: {
            return;
        }
    }

    NodeID node_id = m_node_ids[tag];
    m_node_ids.erase(tag);
    m_tags.erase(node_id);

    m_aabb_tree.remove(tag);
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

    switch (tag.type) {
        case ColliderType::collider_type_mesh: {
            MeshCollider const & col = m_mesh_colliders[tag.id];
            res = move_and_collide<MeshCollider, Triangle>(
                scene, tag, col, movement, transform
            );
            break;
        }
        case ColliderType::collider_type_sphere: {
            SphereCollider const & col = m_sphere_colliders[tag.id];
            res = move_and_collide<SphereCollider, Sphere>(
                scene, tag, col, movement, transform
            );
            break;
        }
        case ColliderType::collider_type_box: {
            BoxCollider const & col = m_box_colliders[tag.id];
            res = move_and_collide<BoxCollider, DiscreteConvexHull<8> >(
                scene, tag, col, movement, transform
            );
            break;
        }
        default: {}
    }
    node.set_global_transform(scene, transform);

    if (transform != start_transform) {
        AABB aabb;
        switch (tag.type) {
            case ColliderType::collider_type_mesh: {
                m_mesh_colliders[tag.id].set_transform(transform);
                aabb = m_mesh_colliders[tag.id].aabb();
                break;
            }
            case ColliderType::collider_type_sphere: {
                aabb = m_sphere_colliders[tag.id].get_shape(transform).aabb();
                break;
            }
            case ColliderType::collider_type_box: {
                aabb = m_box_colliders[tag.id].get_shape(transform).aabb();
                break;
            }
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

ColliderTag PhysicsSystem::create_collider_from_triangles(
        std::vector<glm::vec3> const & triangles,
        Transform const & transform
) {
    ColliderTag tag;
    tag.type = ColliderType::collider_type_mesh;
    tag.id = m_next_mesh_id;
    ++m_next_mesh_id;

    MeshCollider & col = m_mesh_colliders[tag.id];
    col.set_transform(transform);
    col.set_triangles(triangles);

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

    for (auto const & node : model.nodes()) {
        if (node.mesh_index == -1) {
            continue;
        }

        glm::mat4 tform = node.transform.to_matrix();
        auto const & mesh = model.meshes()[node.mesh_index];

        auto end_index = mesh.start_index + mesh.num_indices;
        for (auto i = mesh.start_index; i < end_index; ++i) {
            tris[i] = glm::vec3(tform * glm::vec4(v_buf[i_buf[i]].position, 1.0f));
        }
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

ColliderTag PhysicsSystem::create_box_collider(
    glm::vec3 dimensions,
    glm::vec3 center,
    Transform const & transform
) {
    ColliderTag tag;
    tag.type = ColliderType::collider_type_box;
    tag.id = m_next_box_id;
    ++m_next_box_id;

    m_box_colliders[tag.id] = BoxCollider{dimensions, center};

    m_aabb_tree.insert(
        tag,
        m_box_colliders[tag.id].get_shape(transform).aabb()
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
    thread_local std::vector<ColliderTag> stale_tags;
    stale_tags.resize(0);
    thread_local std::vector<AABB> new_aabbs;
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
                case ColliderType::collider_type_mesh: {
                    MeshCollider & col = m_mesh_colliders[tag.id];
                    col.set_transform(t_curr);
                    AABB aabb = col.aabb();
                    new_aabbs.push_back(aabb);
                    break;
                }
                case ColliderType::collider_type_sphere: {
                    AABB aabb = m_sphere_colliders[tag.id].get_shape(t_curr).aabb();
                    new_aabbs.push_back(aabb);
                    break;
                }
                case ColliderType::collider_type_box: {
                    AABB aabb = m_box_colliders[tag.id].get_shape(t_curr).aabb();
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

void PhysicsSystem::collect_collider_render_data(
    Renderer & renderer,
    Transform const * transforms,
    NodeID selected,
    ColliderRenderData & data
) {
    for (auto & pair : m_mesh_colliders) {
        auto & collider = pair.second;
        if (collider.m_changed) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.type = ColliderType::collider_type_mesh;

            auto const & tris = collider.triangles();
            size_t n = tris.size();

            thread_local std::vector<glm::vec3> lines;
            size_t ln = n == 0 ? 0 : 2 * (n - 1);
            lines.resize(ln);

            size_t li = 0;
            for (size_t i = 0; i + 1 < n; ++i) {
                lines[li] = tris[i];
                ++li;
                lines[li] = tris[i+1];
                ++li;
            }

            if (m_collider_meshes.find(tag) == m_collider_meshes.end()) {
                m_collider_meshes[tag] =
                    renderer.upload_line_mesh(lines.data(), lines.size());
            } else {
                renderer.update_line_mesh(
                    m_collider_meshes.at(tag), lines.data(), lines.size()
                );
            }

            collider.m_changed = false;
        }
    }

    for (auto & pair : m_sphere_colliders) {
        auto & collider = pair.second;
        if (collider.m_changed) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.type = ColliderType::collider_type_sphere;

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
                    renderer.upload_line_mesh(lines.data(), lines.size());
            } else {
                renderer.update_line_mesh(
                    m_collider_meshes.at(tag), lines.data(), lines.size()
                );
            }

            collider.m_changed = false;
        }
    }

    for (auto & pair : m_box_colliders) {
        auto & collider = pair.second;
        if (collider.m_changed) {
            ColliderTag tag;
            tag.id = pair.first;
            tag.type = ColliderType::collider_type_box;

            std::array<glm::vec3, 24> lines;
            glm::vec3 max = collider.center() + 0.5f * collider.dimensions();
            glm::vec3 min = collider.center() - 0.5f * collider.dimensions();

            // bottom
            lines[0] = glm::vec3{min.x, min.y, min.z};
            lines[1] = glm::vec3{min.x, min.y, max.z};

            lines[2] = glm::vec3{min.x, min.y, max.z};
            lines[3] = glm::vec3{max.x, min.y, max.z};

            lines[4] = glm::vec3{max.x, min.y, max.z};
            lines[5] = glm::vec3{max.x, min.y, min.z};

            lines[6] = glm::vec3{max.x, min.y, min.z};
            lines[7] = glm::vec3{min.x, min.y, min.z};

            // top
            lines[8] = glm::vec3{min.x, max.y, min.z};
            lines[9] = glm::vec3{min.x, max.y, max.z};

            lines[10] = glm::vec3{min.x, max.y, max.z};
            lines[11] = glm::vec3{max.x, max.y, max.z};

            lines[12] = glm::vec3{max.x, max.y, max.z};
            lines[13] = glm::vec3{max.x, max.y, min.z};

            lines[14] = glm::vec3{max.x, max.y, min.z};
            lines[15] = glm::vec3{min.x, max.y, min.z};

            // connections
            lines[16] = glm::vec3{min.x, min.y, min.z};
            lines[17] = glm::vec3{min.x, max.y, min.z};

            lines[18] = glm::vec3{min.x, min.y, max.z};
            lines[19] = glm::vec3{min.x, max.y, max.z};

            lines[20] = glm::vec3{max.x, min.y, max.z};
            lines[21] = glm::vec3{max.x, max.y, max.z};

            lines[22] = glm::vec3{max.x, min.y, min.z};
            lines[23] = glm::vec3{max.x, max.y, min.z};

            if (m_collider_meshes.find(tag) == m_collider_meshes.end()) {
                m_collider_meshes[tag] =
                    renderer.upload_line_mesh(lines.data(), lines.size());
            } else {
                renderer.update_line_mesh(
                    m_collider_meshes.at(tag), lines.data(), lines.size()
                );
            }

            collider.m_changed = false;
        }
    }

    if (selected == NO_NODE) {
        data.line_data.resize(m_tags.size());
        size_t i = 0;
        for (auto const & pair : m_tags) {
            data.line_data[i].mesh_id = m_collider_meshes.at(pair.second);
            // NOTE: this is incorrect for spheres, since the collider
            //       transforms the sphere radius according to
            //       compMax(transform.scale) * radius. Sphere radii
            //       are one dimensional, hence this limitation.
            // TODO: resolve this discrepancy
            data.line_data[i].transform =
                transforms[pair.first].to_matrix();
            ++i;
        }
    } else if (m_tags.find(selected) != m_tags.end()){
        data.line_data.resize(1);
        data.line_data[0].mesh_id =
            m_collider_meshes.at(m_tags.at(selected));
        data.line_data[0].transform = transforms[selected].to_matrix();
    }
}
