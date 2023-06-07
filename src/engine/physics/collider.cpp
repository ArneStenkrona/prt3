#include "collider.h"

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
    for (size_t i = 0; i < m_aabbs.size(); ++i) {
        size_t ti = 3 * i;
        bool intersect = AABB::intersect(aabb, m_aabbs[i]);
        if (intersect) {
            triangles.push_back({
                m_triangle_cache[ti],
                m_triangle_cache[ti + 1],
                m_triangle_cache[ti + 2]
            });
        }
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
    m_aabbs.resize(m_triangles.size() / 3);
    m_triangle_cache.resize(m_triangles.size());

    glm::mat4 mat = m_transform.to_matrix();

    glm::vec3 const * vertices = m_triangles.data();
    glm::vec3 * vertex_cache = m_triangle_cache.data();

    for (size_t i = 0; i < m_triangles.size(); ++i) {
        vertex_cache[i] = mat * glm::vec4(vertices[i], 1.0f);
    }

    for (size_t i = 0; i < m_aabbs.size(); ++i) {
        size_t ti = 3 * i;
        glm::vec3 const & a = m_triangle_cache[ti];
        glm::vec3 const & b = m_triangle_cache[ti + 1];
        glm::vec3 const & c = m_triangle_cache[ti + 2];
        glm::vec3 min = glm::min(glm::min(a, b), c);
        glm::vec3 max = glm::max(glm::max(a, b), c);

        m_aabbs[i].lower_bound = min;
        m_aabbs[i].upper_bound = max;
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
