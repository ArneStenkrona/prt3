#ifndef PRT3_NPC_CONTROLLER_H
#define PRT3_NPC_CONTROLLER_H

#include "src/engine/component/script/character_controller.h"

namespace prt3 {

class NPCController : public CharacterController {
public:
    explicit NPCController(Scene & scene, NodeID m_node_id)
        : CharacterController(scene, m_node_id) {}

    explicit NPCController(std::istream &, Scene & scene, NodeID m_node_id)
        : CharacterController(scene, m_node_id) {}

    virtual void on_init(Scene & scene) {
        m_path_dest =
            scene.get_node(node_id()).get_global_transform(scene).position;
        CharacterController::on_init(scene);
    }

    virtual void update_input(Scene & scene, float delta_time) {
        m_state.input = {};
        m_state.input.run = false;

        m_target_id = *(scene.find_nodes_by_tag("player").begin());

        Node & node = scene.get_node(node_id());
        Node const & target = scene.get_node(m_target_id);

        glm::vec3 origin = node.get_global_transform(scene).position;
        glm::vec3 target_pos = target.get_global_transform(scene).position;

        if (glm::distance(target_pos, m_path_dest) > 1.0f) {
            NavigationSystem & sys = scene.navigation_system();
            m_path.clear();
            sys.generate_path(origin, target_pos, m_path);
            m_path_dest = target_pos;
            m_path_index = 0;

        }

        if (m_path_index < m_path.size()) {
            glm::vec3 pos = node.get_global_transform(scene).position;
            glm::vec3 dest = m_path[m_path_index];

            float dist = glm::distance(pos, dest);
            // TODO: get criteria from collider dimensions
            float critera = glm::max(glm::length(m_state.velocity) * delta_time, 1.0f);
            if (dist >= critera && dist != 0.0f) {
                glm::vec3 dir = glm::normalize(dest - pos);
                dir.y = 0.0f;

                m_state.input.direction = dir;
            } else {
                ++m_path_index;
            }
        }
        m_state.state = WALK;
    }

protected:
    NodeID m_target_id = NO_NODE;

    std::vector<glm::vec3> m_path;
    size_t m_path_index;
    glm::vec3 m_path_dest;

REGISTER_SCRIPT_BEGIN(NPCController, npc_controller, 15129599306800160206)
REGISTER_SERIALIZED_FIELD(m_walk_force)
REGISTER_SERIALIZED_FIELD(m_run_force)
REGISTER_SCRIPT_END()
};

} // namespace prt3

#endif // PRT3_NPC_CONTROLLER_H
