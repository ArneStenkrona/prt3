#ifndef PRT3_COLLIDER_TAG_H
#define PRT3_COLLIDER_TAG_H

#include <cstdint>
#include <functional>

namespace prt3 {

namespace ColliderNS {

enum ShapeEnum : uint8_t {
    none,
    mesh,
    sphere,
    box,
    capsule,
    total_num_collider_shape
};

enum TypeEnum : uint8_t {
    collider,
    area
};

} // namespace ColliderShape

typedef ColliderNS::ShapeEnum ColliderShape;
typedef ColliderNS::TypeEnum ColliderType;

typedef uint16_t ColliderID;

typedef uint16_t CollisionLayer;

struct ColliderTag {
    ColliderID id;
    ColliderShape shape;
    ColliderType type;
};

inline bool operator==(ColliderTag const & lhs, ColliderTag const & rhs) {
    return lhs.type == rhs.type && lhs.shape == rhs.shape && lhs.id == rhs.id;
}

inline bool operator!=(ColliderTag const & lhs, ColliderTag const & rhs) {
    return lhs.type != rhs.type || lhs.shape != rhs.shape || lhs.id != rhs.id;
}

} // namespace prt3

namespace std {

template <>
struct hash<prt3::ColliderTag> {
    size_t operator()(prt3::ColliderTag const & t) const {
        return hash<prt3::ColliderType>()(t.type) ^
              (hash<prt3::ColliderShape>()(t.shape) << 1) ^
              (hash<prt3::ColliderID>()(t.id) << 2);
    }
};

} // namespace std

#endif // PRT3_COLLIDER_TAG_H
