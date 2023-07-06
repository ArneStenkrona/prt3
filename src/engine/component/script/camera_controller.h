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

    virtual void on_late_init(Scene & scene) {
        NodeID player_id = *(scene.find_nodes_by_tag("player").begin());
        set_target(player_id);
    }

    virtual void on_init(Scene & scene) {
        scene.get_camera().set_orthographic_projection(true);
    }

    virtual void on_late_update(Scene & scene, float delta_time) {
        Camera & camera = scene.get_camera();
        Transform & cam_tform = camera.transform();
        Input & input = scene.get_input();

        //Mouse
        int x, y;
        input.get_cursor_delta(x, y);
        float dx = (float)x * m_mouse_sensitivity, dy = (float)y * m_mouse_sensitivity;

        m_yaw -= dx;
        m_pitch -= dy;

        // constrainPitch should have its effect here
        if (m_pitch > glm::radians(89.0f)) {
            m_pitch = glm::radians(89.0f);
        }
        if (m_pitch < glm::radians(-89.0f)) {
            m_pitch = glm::radians(-89.0f);
        }

        cam_tform.rotation = glm::quat_cast(
            glm::eulerAngleYXZ(m_yaw, -m_pitch, 0.0f)
        );

        glm::vec3 target_pos{0.0f, 0.0f, 0.0f};
        if (m_target != NO_NODE) {
            Node & t_node = scene.get_node(m_target);
            target_pos = t_node.get_global_transform(scene).position;
        }

        // camera shake;
        glm::vec3 cam_shake{0.0f};
        if (m_screen_shake_elapsed < m_screen_shake_time) {
            float t = 10.0f * m_screen_shake_elapsed;

            float e = m_screen_shake_elapsed;
            float u = m_screen_shake_rampup;
            float d = m_screen_shake_rampdown;
            float mt = m_screen_shake_time;

            float tu = glm::min(e / u, 1.0f);
            float td = glm::min((mt - e) / d, 1.0f);


            float mag = m_screen_shake_magnitude *
                        glm::sin(7.07 * t) * glm::min(tu, td);
            cam_shake = mag * camera.get_right() * glm::sin(t) +
                        mag * camera.get_up() * glm::cos(t);

            m_screen_shake_elapsed += delta_time;
        }

        camera.transform().position =
            target_pos - (m_target_distance * camera.get_front())
            + cam_shake;

        glm::vec3 pos = camera.get_position();
        Transform tform;
        tform.position = pos;
        get_node(scene).set_global_transform(scene, tform);
    }

    void screen_shake(float magnitude,
                      float time,
                      float rampup,
                      float rampdown) {
        m_screen_shake_magnitude = magnitude;
        m_screen_shake_time = time;
        m_screen_shake_rampup = rampup;
        m_screen_shake_rampdown = rampdown;

        m_screen_shake_elapsed = 0.0f;
    }

private:
    float m_yaw;
    float m_pitch;
    float m_mouse_sensitivity = 0.005236f;

    NodeID m_target = NO_NODE;
    float m_target_distance = 25.0f;

    float m_screen_shake_magnitude = 0.0f;
    float m_screen_shake_time = 0.0f;
    float m_screen_shake_elapsed = 0.0f;
    float m_screen_shake_rampup = 0.0f;
    float m_screen_shake_rampdown = 0.0f;

REGISTER_SCRIPT(CameraController, camera_controller, 17005293234220491566)
};

} // namespace prt3

#endif
