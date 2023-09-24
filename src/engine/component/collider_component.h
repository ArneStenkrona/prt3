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
    ColliderComponent(
        Scene & scene,
        NodeID node_id,
        ColliderType type,
        ModelHandle model_handle
    );
    ColliderComponent(
        Scene & scene,
        NodeID node_id,
        ColliderType type,
        Model const & model
    );
    ColliderComponent(
        Scene & scene,
        NodeID node_id,
        ColliderType type,
        std::vector<glm::vec3> && triangles
    );
    ColliderComponent(
        Scene & scene,
        NodeID node_id,
        ColliderType type,
        std::vector<glm::vec3> const & triangles
    );
    ColliderComponent(
        Scene & scene,
        NodeID node_id,
        ColliderType type,
        Sphere const & sphere
    );
    ColliderComponent(
        Scene & scene,
        NodeID node_id,
        ColliderType type,
        Box const & box
    );
    ColliderComponent(
        Scene & scene,
        NodeID node_id,
        ColliderType type,
        Capsule const & capsule
    );
    ColliderComponent(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }
    ColliderTag tag() const { return m_tag; }

    void set_layer(Scene & scene, CollisionLayer layer);
    void set_mask(Scene & scene, CollisionLayer mask);

    void set_collider(Scene & scene, ColliderType type, ModelHandle model_handle);
    void set_collider(Scene & scene, ColliderType type, Model const & model);
    void set_collider(
        Scene & scene,
        ColliderType type,
        std::vector<glm::vec3> && triangles
    );
    void set_collider(
        Scene & scene,
        ColliderType type,
        std::vector<glm::vec3> const & triangles
    );
    void set_collider(Scene & scene, ColliderType type, Sphere const & sphere);
    void set_collider(Scene & scene, ColliderType type, Box const & box);
    void set_collider(Scene & scene, ColliderType type, Capsule const & capsule);

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
    ColliderTag m_tag = { 0, ColliderShape::none, ColliderType::collider };

    void remove(Scene & scene);

    friend class ComponentStorage<ColliderComponent>;
};

}

#endif
