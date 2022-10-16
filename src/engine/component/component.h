#ifndef PRT3_COMPONENT_H
#define PRT3_COMPONENT_H

#include <cstdint>

namespace prt3 {

enum class ComponentType : uint16_t {
    Mesh,
    Material,
    PointLight,
    ScriptSet
};

typedef uint16_t ComponentID;
constexpr ComponentID NO_COMPONENT = -1;

struct ComponentTag {
    ComponentType type;
    ComponentID   id;
};

} // namespace prt3

#endif
