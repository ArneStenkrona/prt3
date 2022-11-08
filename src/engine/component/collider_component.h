#ifndef PRT3_COLLIDER_COMPONENT_H
#define PRT3_COLLIDER_COMPONENT_H

#include "src/engine/physics/collider.h"
#include "src/engine/scene/node.h"
#include "src/engine/rendering/model.h"
#include "src/util/serialization_util.h"

#include <iostream>

namespace prt3 {

class Scene;
template<typename T>
class ComponentStorage;

class ColliderComponent {
public:
    ColliderComponent(Scene & scene, NodeID node_id, Model const & model);
    ColliderComponent(Scene & scene, NodeID node_id, Sphere const & sphere);
    ColliderComponent(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ColliderTag tag() const { return m_tag; }


    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Collider"; }

private:

    NodeID m_node_id;
    ColliderTag m_tag;

    void remove(Scene & scene);

    friend class ComponentStorage<ColliderComponent>;
};

}

#endif
