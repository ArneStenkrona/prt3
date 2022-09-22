#include "collider.h"

using namespace prt3;

void MeshCollider::set_triangles(
    std::vector<Triangle> && triangles) {
    m_triangles = triangles;
    m_aabbs.resize(m_triangles.size());

    for (size_t i = 0; i < m_aabbs.size(); ++i) {
        Triangle const & tri = m_triangles[i];
        glm::vec3 min = glm::min(glm::min(tri.a, tri.b), tri.c);
        glm::vec3 max = glm::max(glm::max(tri.a, tri.b), tri.c);

        m_aabbs[i].lower_bound = min;
        m_aabbs[i].upper_bound = max;
    }

    m_normals.resize(m_triangles.size());
    for (size_t i = 0; i < m_triangles.size(); ++i) {
        glm::vec3 a = m_triangles[i].b - m_triangles[i].a;
        glm::vec3 b = m_triangles[i].c - m_triangles[i].a;
        m_normals[i] = glm::normalize(glm::cross(a, b));
    }
}

void MeshCollider::collect_triangles(
    AABB const & aabb,
    std::vector<Triangle> & triangles) const {
    for (size_t i = 0; i < m_triangles.size(); ++i) {
        bool intersect = AABB::intersect(aabb, m_aabbs[i]);
        if (intersect) {
            triangles.push_back(m_triangles[i]);
        }
    }
}

void MeshCollider::collect_triangles(
    AABB const & aabb,
    glm::vec3 const & movement_vector,
    std::vector<Triangle> & triangles) const {
    for (size_t i = 0; i < m_triangles.size(); ++i) {
        bool intersect = AABB::intersect(aabb, m_aabbs[i]);
        bool towards = glm::dot(movement_vector, m_normals[i]) <= 0.0f;
        if (intersect && towards) {
            triangles.push_back(m_triangles[i]);
        }
    }
}
