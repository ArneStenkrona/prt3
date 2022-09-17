#include "aabb.h"

bool prt3::AABB::intersect(
    AABB const & a,
    AABB const & b) {
    return !(a.lower_bound.x > b.upper_bound.x ||
             a.upper_bound.x < b.lower_bound.x ||
             a.lower_bound.y > b.upper_bound.y ||
             a.upper_bound.y < b.lower_bound.y ||
             a.lower_bound.z > b.upper_bound.z ||
             a.upper_bound.z < b.lower_bound.z);
}

void prt3::AABB::intersect(
    AABB const & a,
    std::vector<AABB> const & b,
    std::vector<bool> & result) {
    result.resize(b.size());
    for (size_t i = 0; i < b.size(); ++i) {
        result[i] = !(a.lower_bound.x > b[i].upper_bound.x ||
                      a.upper_bound.x < b[i].lower_bound.x ||
                      a.lower_bound.y > b[i].upper_bound.y ||
                      a.upper_bound.y < b[i].lower_bound.y ||
                      a.lower_bound.z > b[i].upper_bound.z ||
                      a.upper_bound.z < b[i].lower_bound.z);
    }
}

// From "Realtime Collision Detection" by Christer Ericson
bool prt3::AABB::intersect_ray(
    AABB const & a,
    glm::vec3 const & origin,
    glm::vec3 const & direction,
    float max_distance) {
    float tmin = 0.0f; // set to -FLT_MAX to get first hit on line
    float tmax = max_distance; // set to max distance ray can travel (for segment)
    // For all three slabs
    for (int i = 0; i < 3; i++) {
        if (std::abs(direction[i]) < 0.00001 /*epsilon*/) {
            // Ray is parallel to slab. No hit if origin not within slab
            if (origin[i] < a.lower_bound[i] || origin[i] > a.upper_bound[i]) {
                return false;
            }
        } else {
            // Compute intersection t value of ray with near and far plane of
            // slab
            float ood = 1.0f / direction[i];
            float t1 = (a.lower_bound[i] - origin[i]) * ood;
            float t2 = (a.upper_bound[i] - origin[i]) * ood;
            // Make t1 be intersection with near plane, t2 with far plane
            if (t1 > t2) {
                auto temp = t1;
                t1 = t2;
                t2 = temp;
            }
            // Compute the intersection of slab intersection intervals
            if (t1 > tmin) tmin = t1;
            if (t2 < tmax) tmax = t2;
            // Exit with no collision as soon as slab intersection becomes empty
            if (tmin > tmax) {
                return false;
            }
        }
    }
    // Ray intersects all 3 slabs. Return point (q) and intersection t value (tmin)
    //q = p + d * tmin;

    return true;
}


bool prt3::AABB::contains(AABB const & other) {
    return lower_bound.x <= other.lower_bound.x &&
           lower_bound.y <= other.lower_bound.y &&
           lower_bound.z <= other.lower_bound.z &&
           upper_bound.x >= other.upper_bound.x &&
           upper_bound.y >= other.upper_bound.y &&
           upper_bound.z >= other.upper_bound.z;
}

prt3::AABB & prt3::AABB::operator+=(
    AABB const & rhs) {
    lower_bound = glm::min(lower_bound, rhs.lower_bound);
    upper_bound = glm::max(upper_bound, rhs.upper_bound);
    return *this;
}

float prt3::AABB::area() const {
    glm::vec3 d = upper_bound - lower_bound;
    return 2.0f * (d.x * d.y + d.y * d.z + d.z * d.x);
}
float prt3::AABB::volume() const {
    glm::vec3 d = upper_bound - lower_bound;
    return d.x * d.y * d.z;
}