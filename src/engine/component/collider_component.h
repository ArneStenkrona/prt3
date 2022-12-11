#ifndef PRT3_COLLIDER_COMPONENT_H
#define PRT3_COLLIDER_COMPONENT_H

#include "src/engine/physics/collider.h"
#include "src/engine/scene/node.h"
#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/util/serialization_util.h"
#include "src/util/uuid.h"

#include <iostream>

namespace prt3 {

class Scene;
template<typename T>
class ComponentStorage;

class ColliderComponent {
public:
    ColliderComponent(Scene & scene, NodeID node_id);
    ColliderComponent(Scene & scene, NodeID node_id, ModelHandle model_handle);
    ColliderComponent(Scene & scene, NodeID node_id, Model const & model);
    ColliderComponent(
        Scene & scene,
        NodeID node_id,
        std::vector<glm::vec3> && triangles
    );
    ColliderComponent(
        Scene & scene,
        NodeID node_id,
        std::vector<glm::vec3> const & triangles
    );
    ColliderComponent(Scene & scene, NodeID node_id, Sphere const & sphere);
    ColliderComponent(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ColliderTag tag() const { return m_tag; }

    void set_collider(Scene & scene, ModelHandle model_handle);
    void set_collider(Scene & scene, Model const & model);
    void set_collider(
        Scene & scene,
        std::vector<glm::vec3> && triangles
    );
    void set_collider(
        Scene & scene,
        std::vector<glm::vec3> const & triangles
    );
    void set_collider(Scene & scene, Sphere const & sphere);

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    void deserialize(
        std::istream & in,
        Scene & scene
    );

    static char const * name() { return "Collider"; }
    static constexpr UUID uuid = 14156805678675640982ull;

private:

    NodeID m_node_id;
    ColliderTag m_tag = { collider_type_none, 0 };

    void remove(Scene & scene);

    friend class ComponentStorage<ColliderComponent>;
};

}

#endif
