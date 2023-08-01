#ifndef PRT3_PLAYER_CONTROLLER_H
#define PRT3_PLAYER_CONTROLLER_H

#include "src/engine/component/script/character_controller.h"
#include "src/engine/scene/scene_manager.h"

namespace prt3 {

class PlayerController : public CharacterController {
public:
    explicit PlayerController(Scene & scene, NodeID m_node_id)
        : CharacterController(scene, m_node_id) {}

    explicit PlayerController(std::istream &, Scene & scene, NodeID m_node_id)
        : CharacterController(scene, m_node_id) {}

    virtual void on_init(Scene & scene) {
        set_tag(scene, "player");
        CharacterController::on_init(scene);
        scene.selected_node() = node_id();

        thread_local std::vector<NodeID> queue;
        queue.emplace_back(node_id());
        while (!queue.empty()) {
            NodeID id = queue.back();
            queue.pop_back();

            scene.scene_manager().exclude_node_from_fade(id);

            for (NodeID child_id : scene.get_node(id).children_ids()) {
                queue.push_back(child_id);
            }
        }
    }

    virtual void update_input(Scene & scene, float /*delta_time*/) {
        Input & input = scene.get_input();

        m_state.input.run = input.get_key(KeyCode::KEY_CODE_LEFT_SHIFT);
        m_state.input.attack = input.get_key_down(KeyCode::KEY_CODE_ENTER);
        m_state.input.jump = input.get_key_down(KeyCode::KEY_CODE_SPACE);

        glm::vec2 raw_input_dir{0.0f};

        if (input.get_key(KeyCode::KEY_CODE_W)) {
            raw_input_dir += glm::vec2{0.0f, 1.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_S)) {
            raw_input_dir -= glm::vec2{0.0f, 1.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_A)) {
            raw_input_dir -= glm::vec2{1.0f, 0.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_D)) {
            raw_input_dir += glm::vec2{1.0f, 0.0f};
        }

        m_state.input.direction = glm::vec3{ 0.0f };
        // project input according to camera
        if (raw_input_dir != glm::vec2{0.0f}) {
            // compute look direction
            Camera const & camera = scene.get_camera();
            glm::vec3 c_proj_x = camera.get_right();
            constexpr float quarter_pi = glm::pi<float>() / 4.0f;
            glm::vec3 c_proj_y =
                glm::abs(glm::asin(-camera.get_front().y)) <= quarter_pi ?
                camera.get_front() :
                glm::sign(-camera.get_front().y) * camera.get_up();

            m_state.input.direction = glm::normalize(
                raw_input_dir.x * glm::vec3{c_proj_x.x, 0.0f, c_proj_x.z} +
                raw_input_dir.y * glm::vec3{c_proj_y.x, 0.0f, c_proj_y.z}
            );
        }

        if (m_state.grounded) {
            m_state.input.last_grounded_direction = m_state.input.direction;
            m_state.input.last_grounded_run = m_state.input.run;
            m_state.jump_count = 0;
            // project movement onto ground
            m_state.input.direction = m_state.input.direction -
                                glm::dot(m_state.input.direction,
                                         m_state.ground_normal) *
                                    m_state.ground_normal;
            if (m_state.input.direction != glm::vec3{0.0f}) {
                m_state.input.direction = glm::normalize(m_state.input.direction);
            }
        }
    }

REGISTER_SCRIPT_BEGIN(PlayerController, player_controller, 3968611710651155566)
REGISTER_SERIALIZED_FIELD(m_walk_force)
REGISTER_SERIALIZED_FIELD(m_run_force)
REGISTER_SCRIPT_END()
};

} // namespace prt3

#endif // PRT3_PLAYER_CONTROLLER_H
