#ifndef PRT3_CAMERA_CONTROLLER_H
#define PRT3_CAMERA_CONTROLLER_H

#include "src/engine/scene/script/script.h"
#include "src/engine/scene/scene.h"

#include <glm/gtx/euler_angles.hpp>

#include <iostream>

namespace prt3 {

class CameraController : public Script {
public:
    explicit CameraController(Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    void set_target(NodeID target) {
        m_target = target;
    }

    inline float get_target_distance() const { return m_target_distance; }
    inline void set_target_distance(float distance) { m_target_distance = distance; }

    virtual void on_init() {
    }

    virtual void on_late_update(float delta_time) {
        Camera & camera = scene().get_camera();
        Transform & cam_tform = camera.transform();
        Input & input = scene().get_input();

        if (input.get_key_down(KeyCode::KEY_CODE_TAB)) {
            m_free_cam_mode = !m_free_cam_mode;
            camera.set_orthographic_projection(!m_free_cam_mode);
        }

        if (!m_free_cam_mode || input.get_key(KeyCode::KEY_CODE_MOUSE_LEFT)) {
            //Mouse
            int x, y;
            input.get_cursor_delta(x, y);
            float dx = (float)x * m_mouse_sensitivity, dy = (float)y * m_mouse_sensitivity;

            m_yaw -= dx;
            m_pitch -= dy;

            // constrainPitch should have its effect here
            if (m_pitch > 89.0f) {
                m_pitch = 89.0f;
            }
            if (m_pitch < -89.0f) {
                m_pitch = -89.0f;
            }

            cam_tform.rotation = glm::quat_cast(
                glm::eulerAngleYXZ(glm::radians(m_yaw), glm::radians(-m_pitch), 0.0f)
            );
        }

        if (m_free_cam_mode) {
            //Movement
            float camera_speed = m_movement_speed * delta_time;

            glm::vec3 front = camera.get_front();
            glm::vec3 right = camera.get_right();
            if (input.get_key(KeyCode::KEY_CODE_W))
                cam_tform.position += front * camera_speed;
            if (input.get_key(KeyCode::KEY_CODE_S))
                cam_tform.position -= front * camera_speed;
            if (input.get_key(KeyCode::KEY_CODE_A))
                cam_tform.position -= right * camera_speed;
            if (input.get_key(KeyCode::KEY_CODE_D))
                cam_tform.position += right * camera_speed;
            if (input.get_key(KeyCode::KEY_CODE_SPACE))
                cam_tform.position += Camera::WORLD_UP * camera_speed;
            if (input.get_key(KeyCode::KEY_CODE_LCTRL))
                cam_tform.position -= Camera::WORLD_UP * camera_speed;
        } else {
            glm::vec3 target_pos{0.0f, 0.0f, 0.0f};
            if (m_target != NO_NODE) {
                Node & t_node = scene().get_node(m_target);
                target_pos = t_node.get_global_transform().position;
            }
            camera.transform().position =
                target_pos - (m_target_distance * camera.get_front());
        }

        glm::vec3 pos = camera.get_position();
        Transform tform;
        tform.position = pos;
        node().set_global_transform(tform);
    }
private:
    float m_yaw;
    float m_pitch;
    float m_movement_speed = 25.0f;
    float m_mouse_sensitivity = 0.3f;


    NodeID m_target = NO_NODE;
    float m_target_distance = 25.0f;

    bool m_free_cam_mode = true;
};

} // namespace prt3

#endif
