#ifndef DDS_ID_H
#define DDS_ID_H

#include <cstdint>

namespace dds {

typedef uint32_t DDSID;
typedef DDSID NPCID;
typedef DDSID ObjectID;

enum IDType : DDSID {
    dds_id_type_npc,
    dds_id_type_player,
    dds_id_type_item,
    dds_id_type_object,
};

struct AnyID {
    IDType type;
    DDSID id;
};

inline bool operator==(AnyID const & lhs, AnyID const & rhs) {
    return lhs.type == rhs.type &&
           lhs.id == rhs.id;
}

inline bool operator!=(AnyID const & lhs, AnyID const & rhs) {
    return lhs.type != rhs.type ||
           lhs.id != rhs.id;
}

} // namespace dds

#endif  // DDS_ID_H
