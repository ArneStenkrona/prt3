#ifndef PRT3_FOLLOW_PLAYER_H
#define PRT3_FOLLOW_PLAYER_H

#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"
#include "src/util/log.h"

#include <vector>

namespace prt3 {

class FollowPlayer : public Script {
public:
    explicit FollowPlayer(Scene & scene, NodeID node_id)
        : Script(scene, node_id) {}

    explicit FollowPlayer(std::istream &, Scene & scene, NodeID node_id)
        : Script(scene, node_id) {}

    virtual void on_init(Scene & scene) {
        m_target_id = scene.find_node_by_tag("player");
        m_path_dest =
            scene.get_node(node_id()).get_global_transform(scene).position;
    }

    virtual void on_update(Scene & scene, float delta_time) {
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
            if (dist >= m_speed * delta_time && dist != 0.0f) {
                glm::vec3 dir = glm::normalize(dest - pos);

                node.translate_node(scene, dir * m_speed * delta_time);
            } else {
                ++m_path_index;
            }
        }
    }

private:
    NodeID m_target_id = NO_NODE;

    float m_speed = 4.0f;

    std::vector<glm::vec3> m_path;
    size_t m_path_index;
    glm::vec3 m_path_dest;

REGISTER_SCRIPT(FollowPlayer, follow_player, 15077232273564419390)
};

} // namespace prt3

#endif // PRT3_FOLLOW_PLAYER_H
