#ifndef PRT3_SCENE_CONTENT_H
#define PRT3_SCENE_CONTENT_H

#include "src/engine/scene/node.h"
#include "src/engine/component/component_manager.h"
#include "src/engine/physics/physics_system.h"

#include <vector>

namespace prt3 {

class SceneContent {
public:
private:
    Camera m_camera;

    static constexpr NodeID m_root_id = 0;
    std::vector<Node> m_nodes;
    std::vector<NodeName> m_node_names;

    DirectionalLight m_directional_light;
    bool m_directional_light_on = false;

    AmbientLight m_ambient_light;

    TransformCache m_transform_cache;

    ComponentManager m_component_manager;
    PhysicsSystem m_physics_system;
};

} // namespace prt3

#endif
