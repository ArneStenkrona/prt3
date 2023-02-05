#ifndef PRT3_CHARACTER_CONTROLLER_H
#define PRT3_CHARACTER_CONTROLLER_H

#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"

#include "src/engine/rendering/model.h"
#include "src/engine/geometry/shapes.h"

#include "src/engine/physics/collider.h"
#include "src/engine/physics/gjk.h"

#include <utility>
#include <iostream>

namespace prt3 {

class CharacterController : public Script {
public:
    explicit CharacterController(Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    virtual void on_init(Scene & scene) {
        add_tag(scene, "player");
    }

    virtual void on_update(Scene & scene, float delta_time) {
        Camera & camera = scene.get_camera();
        Input & input = scene.get_input();

        float speed = 20.0f * delta_time;

        glm::vec3 raw_input{ 0.0f, 0.0f, 0.0f };
        if (controllable) {
            if (input.get_key(KeyCode::KEY_CODE_W)) {
                raw_input += glm::vec3{1.0f, 0.0f, 0.0f};
            }
            if (input.get_key(KeyCode::KEY_CODE_S)) {
                raw_input -= glm::vec3{1.0f, 0.0f, 0.0f};
            }
            if (input.get_key(KeyCode::KEY_CODE_A)) {
                raw_input -= glm::vec3{0.0f, 0.0f, 1.0f};
            }
            if (input.get_key(KeyCode::KEY_CODE_D)) {
                raw_input += glm::vec3{0.0f, 0.0f, 1.0f};
            }
            if (input.get_key(KeyCode::KEY_CODE_E)) {
                raw_input += glm::vec3{0.0f, 1.0f, 0.0f};
            }
            if (input.get_key(KeyCode::KEY_CODE_Q)) {
                raw_input -= glm::vec3{0.0f, 1.0f, 0.0f};
            }
        }
        glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
        // project input according to camera
        if (raw_input != glm::vec3{0.0f}) {
            // compute look direction
            glm::vec3 cf = camera.get_front();
            glm::vec3 cr = camera.get_right();
            glm::vec3 dir = glm::normalize(
                                raw_input.x * glm::vec3{cf.x, 0.0f, cf.z} +
                                raw_input.z * glm::vec3{cr.x, 0.0f, cr.z} +
                                glm::vec3{0.0f, raw_input.y, 0.0f}
                            );

            translation = speed * dir;

            glm::vec3 raw_look_dir = glm::vec3{dir.x, 0.0f, dir.z};
            if (raw_look_dir != glm::vec3{0.0f}) {
                glm::vec3 look_dir = glm::normalize(raw_look_dir);
                glm::quat rot = glm::rotation(glm::vec3{0.0f, 0.0f, 1.0f}, look_dir);
                get_node(scene).set_global_rotation(scene, rot);
            }
        }

        translation += m_gravity_velocity * m_gravity_direction;

        auto res = get_node(scene).move_and_collide(scene, translation);
        if (res.grounded) {
            m_gravity_velocity = 0.0f;
            m_gravity_direction = -res.ground_normal;
        } else {
            m_gravity_direction = glm::vec3{0.0f, -1.0f, 0.0f};
        }
        m_gravity_velocity = glm::min(
            m_gravity_velocity + gravity_constant * delta_time,
            terminal_velocity * delta_time
        );
    }

private:
    static constexpr float gravity_constant = 2.0f;
    static constexpr float terminal_velocity = 35.0f;
    float m_gravity_velocity = 0.0f;
    glm::vec3 m_gravity_direction{0.0f, -1.0f, 0.0f};
    bool controllable = true;

REGISTER_SCRIPT(CharacterController, character_controller, 7387722065150816170)
};

} // namespace prt3

#endif
