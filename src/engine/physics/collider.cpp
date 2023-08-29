#include "collider.h"

#include "src/util/log.h"

#include <cassert>

using namespace prt3;

void MeshCollider::set_triangles(
    std::vector<glm::vec3> && triangles) {
    assert(triangles.size() % 3 == 0);
    m_triangles = triangles;
    update_triangle_cache();
    m_changed = true;
}

void MeshCollider::set_triangles(
    std::vector<glm::vec3> const & triangles) {
    assert(triangles.size() % 3 == 0);
    m_triangles = triangles;
    update_triangle_cache();
    m_changed = true;
}

void MeshCollider::collect_triangles(
    AABB const & aabb,
    std::vector<Triangle> & triangles) const {

    thread_local std::vector<ColliderTag> tags;
    tags.clear();
    ColliderTag dummy;
    dummy.id = std::numeric_limits<ColliderID>::max();
    dummy.type = ColliderType::collider;
    dummy.shape = ColliderShape::mesh;
    m_aabb_tree.query(
        dummy,
        1,
        aabb,
        tags
    );

    size_t ti = triangles.size();
    triangles.resize(ti + 3 * tags.size());
    for (ColliderTag tag : tags) {
        triangles[ti] = {
            m_triangle_cache[3*tag.id],
            m_triangle_cache[3*tag.id + 1],
            m_triangle_cache[3*tag.id + 2]
        };
        ++ti;
    }
}

void MeshCollider::set_transform(Transform const & transform) {
    if (transform == m_transform) {
        return;
    }
    m_transform = transform;
    update_triangle_cache();
}

void MeshCollider::update_triangle_cache() {
    m_triangle_cache.resize(m_triangles.size());

    glm::mat4 mat = m_transform.to_matrix();

    glm::vec3 const * vertices = m_triangles.data();
    glm::vec3 * vertex_cache = m_triangle_cache.data();

    for (size_t i = 0; i < m_triangles.size(); ++i) {
        vertex_cache[i] = mat * glm::vec4(vertices[i], 1.0f);
    }

    size_t n_triangles = m_triangles.size() / 3;
    assert(n_triangles < std::numeric_limits<uint16_t>::max());
    for (size_t i = 0; i < n_triangles; ++i) {
        size_t ti = 3 * i;
        glm::vec3 const & a = m_triangle_cache[ti];
        glm::vec3 const & b = m_triangle_cache[ti + 1];
        glm::vec3 const & c = m_triangle_cache[ti + 2];
        glm::vec3 min = glm::min(glm::min(a, b), c);
        glm::vec3 max = glm::max(glm::max(a, b), c);

        AABB aabb;
        aabb.lower_bound = min;
        aabb.upper_bound = max;
        // TODO: provide more general aabb tree api that does not care about
        //       tags, shapes, etc.
        m_aabb_tree.insert(
            ColliderTag{static_cast<ColliderID>(i),
                        ColliderShape::mesh,
                        ColliderType::collider},
            1,
            aabb
        );
    }

    if (m_triangle_cache.empty()) {
        m_aabb = {};
        return;
    } else {
        m_aabb.lower_bound = m_triangle_cache[0];
        m_aabb.upper_bound = m_triangle_cache[0];
        for (glm::vec3 vertex : m_triangle_cache) {
            m_aabb.lower_bound = glm::min(m_aabb.lower_bound, vertex);
            m_aabb.upper_bound = glm::max(m_aabb.upper_bound, vertex);
        }
    }
}
