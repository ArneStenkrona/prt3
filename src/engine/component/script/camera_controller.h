#ifndef PRT3_CAMERA_CONTROLLER_H
#define PRT3_CAMERA_CONTROLLER_H

#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"

#include <glm/gtx/euler_angles.hpp>

#include <iostream>
#include <cmath>

namespace prt3 {

class CameraController : public Script {
public:
    explicit CameraController(Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    explicit CameraController(std::istream &, Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    void set_target(NodeID target) {
        m_target = target;
    }

    inline float get_target_distance() const { return m_target_distance; }
    inline void set_target_distance(float distance) { m_target_distance = distance; }

    inline void set_direction(glm::vec3 direction) {
        m_pitch = glm::asin(-direction.y);
        m_yaw = std::atan2(direction.x, direction.z);
    }

    inline float const & yaw() const { return m_yaw; }
    inline float const & pitch() const { return m_pitch; }

    inline float & yaw() { return m_yaw; }
    inline float & pitch() { return m_pitch; }

    void rotate_with_mouse(Scene & scene);

    void orient_camera(Scene & scene);

    virtual void on_late_start(Scene & scene) {
        orient_camera(scene);
    }

    virtual void on_late_update(Scene & scene, float delta_time);

    void screen_shake(float magnitude,
                      float time,
                      float rampup,
                      float rampdown);

private:
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    float m_mouse_sensitivity = 0.005236f;

    NodeID m_target = NO_NODE;
    float m_target_distance = 20.0f;

    float m_screen_shake_magnitude = 0.0f;
    float m_screen_shake_time = 0.0f;
    float m_screen_shake_elapsed = 0.0f;
    float m_screen_shake_rampup = 0.0f;
    float m_screen_shake_rampdown = 0.0f;

REGISTER_SCRIPT(CameraController, camera_controller, 17005293234220491566)
};

} // namespace prt3

#endif
