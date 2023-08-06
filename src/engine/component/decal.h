#ifndef PRT3_DECAL_H
#define PRT3_DECAL_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/texture_manager.h"
#include "src/util/uuid.h"

namespace prt3 {

class Scene;

template<typename T>
class ComponentStorage;

class EditorContext;

template<typename T>
void inner_show_component(EditorContext &, NodeID);

class ComponentManager;

class Decal {
public:
    Decal(Scene & scene, NodeID node_id);
    Decal(Scene & scene, NodeID node_id, ResourceID texture_id);
    Decal(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }

    ResourceID const & texture_id() const { return m_texture_id; }
    ResourceID & texture_id() { return m_texture_id; }

    glm::vec3 const & dimensions() const { return m_dimensions; }
    glm::vec3 & dimensions() { return m_dimensions; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Decal"; }
    static constexpr UUID uuid = 8259144661757346850ull;

private:
    NodeID m_node_id;

    ResourceID m_texture_id = NO_RESOURCE;

    glm::vec3 m_dimensions = glm::vec3{1.0f};

    void remove(Scene & /*scene*/) {}

    friend class ComponentStorage<Decal>;
    friend class ComponentManager;
    friend void inner_show_component<Decal>(EditorContext &, NodeID);
};

} // namespace prt3

#endif // PRT3_DECAL_H