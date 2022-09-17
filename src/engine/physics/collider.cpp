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

        // // min
        // min.x = tri.b.x < min.x ? tri.b.x : min.x;
        // min.x = tri.c.x < min.x ? tri.c.x : min.x;

        // min.y = tri.b.y < min.y ? tri.b.y : min.y;
        // min.y = tri.c.y < min.y ? tri.c.y : min.y;

        // min.z = tri.b.z < min.z ? tri.b.z : min.z;
        // min.z = tri.c.z < min.z ? tri.c.z : min.z;

        // // max
        // max.x = tri.b.x > max.x ? tri.b.x : max.x;
        // max.x = tri.c.x > max.x ? tri.c.x : max.x;

        // max.y = tri.b.y > max.y ? tri.b.y : max.y;
        // max.y = tri.c.y > max.y ? tri.c.y : max.y;

        // max.z = tri.b.z > max.z ? tri.b.z : max.z;
        // max.z = tri.c.z > max.z ? tri.c.z : max.z;

        m_aabbs[i].lower_bound = min;
        m_aabbs[i].upper_bound = max;
    }
}

void MeshCollider::collect_triangles(
    AABB const & aabb,
    std::vector<Triangle> & triangles) const {
    std::vector<bool> intersections;
    AABB::intersect(aabb, m_aabbs, intersections);

    for (size_t i = 0; i < intersections.size(); ++i) {
        if (intersections[i]) {
            triangles.push_back(m_triangles[i]);
        }
    }
}