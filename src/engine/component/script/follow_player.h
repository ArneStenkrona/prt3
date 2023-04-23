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
    }

    virtual void on_update(Scene & scene, float delta_time) {
        bool gen = false;

        if (m_path.size() <= m_path_index || m_path_index >= 1) {
            gen = true;
        }

        bool path = false;
        if (gen) {
            m_path_index = 0;
            path = get_path(scene);
        }

        Node & node = scene.get_node(node_id());
        glm::vec3 pos = node.get_global_transform(scene).position;
        glm::vec3 dest;
        if (m_path_index < m_path.size()) {

            dest = m_path[m_path_index];

        } else if (path) {
            Node const & target = scene.get_node(m_target_id);
            dest = target.get_global_transform(scene).position;
        }

        float dist = glm::distance(pos, dest);
        if (dist < m_speed * delta_time) {
            ++m_path_index;
        }

        if (dist > 0.0f) {
            glm::vec3 dir = glm::normalize(dest - pos);

            node.translate_node(scene, dir * m_speed * delta_time);
        }
    }

private:
    NodeID m_target_id = NO_NODE;

    float m_speed = 8.0f;

    std::vector<glm::vec3> m_path;
    size_t m_path_index;

    bool get_path(Scene & scene) {
        NavigationSystem & sys = scene.navigation_system();

        Node const & node = scene.get_node(node_id());
        Node const & target = scene.get_node(m_target_id);

        glm::vec3 origin = node.get_global_transform(scene).position;
        glm::vec3 dest = target.get_global_transform(scene).position;

        m_path.clear();
        return sys.generate_path(origin, dest, m_path);
    }

REGISTER_SCRIPT(FollowPlayer, follow_player, 15077232273564419390)
};

} // namespace prt3

#endif // PRT3_FOLLOW_PLAYER_H
