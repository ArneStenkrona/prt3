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

    virtual void on_update(Scene & scene, float delta_time) {
        if (m_target_id == NO_NODE) {
            m_target_id = *(scene.find_nodes_by_tag("player").begin());
            m_path_dest =
                scene.get_node(node_id()).get_global_transform(scene).position;
        }

        Node & node = scene.get_node(node_id());
        Node const & target = scene.get_node(m_target_id);

        glm::vec3 origin = node.get_global_transform(scene).position;
        glm::vec3 target_pos = target.get_global_transform(scene).position;

        if (glm::distance(target_pos, m_path_dest) > 1.0f) {
            NavigationSystem & sys = scene.navigation_system();
            sys.generate_path(origin, target_pos, m_path);
            m_path_dest = target_pos;
            m_path_index = 0;
        }

        if (m_path_index < m_path.size()) {
            glm::vec3 pos = node.get_global_transform(scene).position;
            glm::vec3 dest = m_path[m_path_index];

            float dist = glm::distance(pos, dest);
            // TODO: get criteria from collider dimensions
            float critera = glm::max(m_speed * delta_time, 1.0f);
            if (dist >= critera && dist != 0.0f) {
                glm::vec3 dir = glm::normalize(dest - pos);

                glm::vec3 look_dir{dir.x, 0.0f, dir.z};

                glm::quat rot =
                    glm::rotation(glm::vec3{0.0f, 0.0f, 1.0f}, look_dir);
                node.set_global_rotation(scene, rot);

                if (scene.has_component<ColliderComponent>(node_id())) {
                    scene.physics_system().move_and_collide(
                        scene,
                        node_id(),
                        dir * m_speed * delta_time
                    );
                } else {
                    node.translate_node(scene, dir * m_speed * delta_time);
                }
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
