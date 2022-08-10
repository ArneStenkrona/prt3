#ifndef PRT3_CHARACTER_CONTROLLER_H
#define PRT3_CHARACTER_CONTROLLER_H

#include "src/engine/scene/script/script.h"
#include "src/engine/scene/scene.h"

#include <iostream>

namespace prt3 {

class CharacterController : public Script {
public:
    explicit CharacterController(Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    virtual void on_init() {
    }
    virtual void on_update(float delta_time) {
        Camera & camera = scene().get_camera();
        Input & input = scene().get_input();

        glm::vec3 & position = node().local_transform().position;

        float speed = 10.0f * delta_time;

        glm::vec2 raw_input{ 0.0f, 0.0f };
        if (input.get_key(KeyCode::KEY_CODE_W)) {
            raw_input += glm::vec2{1.0f, 0.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_S)) {
            raw_input -= glm::vec2{1.0f, 0.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_A)) {
            raw_input -= glm::vec2{0.0f, 1.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_D)) {
            raw_input += glm::vec2{0.0f, 1.0f};
        }
        glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
        // project input according to camera
        if (glm::length2(raw_input) > 0.0f) {
            // compute look direction
            glm::vec3 cf = camera.get_front();
            glm::vec3 cr = camera.get_right();
            translation =
                speed *
                glm::normalize(raw_input.x * glm::vec3{cf.x, 0.0f, cf.z} + raw_input.y * glm::vec3{cr.x, 0.0f, cr.z});
        }
        position += translation;

    }
private:
};

} // namespace prt3

#endif
