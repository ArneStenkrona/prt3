#ifndef PRT3_COLLIDER_H
#define PRT3_COLLIDER_H

#include "src/engine/physics/aabb.h"
#include "src/engine/geometry/shapes.h"

#include <vector>

namespace prt3 {

struct Collision {
    glm::vec3 normal;
    float penetration_depth;
    // glm::vec3 impluse;
    bool collided = false;
    // float t = std::numeric_limits<float>::max();
    // TODO: collided nodes
};

enum ColliderType : uint8_t {
    collider_type_none,
    collider_type_mesh,
    collider_type_sphere
};

typedef uint16_t ColliderID;

struct ColliderTag {
    ColliderType type;
    ColliderID   id;
};

inline bool operator==(const ColliderTag& lhs, const ColliderTag& rhs)
{
    return lhs.type == rhs.type && lhs.id == rhs.id;
}

class MeshCollider {
public:
    void set_triangles(std::vector<Triangle> && triangles);
    void collect_triangles(AABB const & aabb,
                           std::vector<Triangle> & triangles) const;
    void collect_triangles(AABB const & aabb,
                           glm::vec3 const & movement_vector,
                           std::vector<Triangle> & triangles) const;
private:
    std::vector<Triangle> m_triangles;
    std::vector<AABB> m_aabbs;
    std::vector<glm::vec3> m_normals;

};

class SphereCollider {
public:
    SphereCollider() {}
    SphereCollider(Sphere const & sphere)
     : m_base_shape{sphere} {}

    Sphere get_shape(Transform const &  transform) const {
        glm::mat4 mat = transform.to_matrix();
        return { glm::vec3{mat * glm::vec4{m_base_shape.position, 1.0f}},
                 m_base_shape.radius };
    }
    Sphere const & base_shape() const { return m_base_shape; }
private:
    Sphere m_base_shape;
};

} // namespace prt3

namespace std {
  template <>
  struct hash<prt3::ColliderTag> {
    size_t operator()(prt3::ColliderTag const & t) const {
      return hash<prt3::ColliderType>()(t.type) ^
             (hash<prt3::ColliderID>()(t.id) << 1);
    }
  };

}


#endif
