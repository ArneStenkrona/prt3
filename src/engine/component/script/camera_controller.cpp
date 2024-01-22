#include "camera_controller.h"
#include "src/engine/scene/scene.h"

#include <glm/gtx/euler_angles.hpp>

#include <iostream>
#include <cmath>

using namespace prt3;

void CameraController::rotate_with_mouse(Scene & scene) {
    Camera & camera = scene.get_camera();
    Input & input = scene.get_input();

    //Mouse
    int x, y;
    input.get_cursor_delta(x, y);
    float dx = (float)x * m_mouse_sensitivity, dy = (float)y * m_mouse_sensitivity;

    m_yaw -= dx;
    m_pitch -= dy;

    if (m_pitch > glm::radians(89.0f)) {
        m_pitch = glm::radians(89.0f);
    }

    if (m_pitch < glm::radians(-89.0f)) {
        m_pitch = glm::radians(-89.0f);
    }

    camera.transform().rotation = glm::quat_cast(
        glm::eulerAngleYXZ(m_yaw, -m_pitch, 0.0f)
    );
}

void CameraController::orient_camera(Scene & scene) {
    Camera & camera = scene.get_camera();

    camera.transform().rotation = glm::quat_cast(
        glm::eulerAngleYXZ(m_yaw, -m_pitch, 0.0f)
    );

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
    }

    glm::vec3 target_pos{0.0f, 0.0f, 0.0f};
    if (m_target != NO_NODE) {
        Node & t_node = scene.get_node(m_target);
        target_pos = t_node.get_global_transform(scene).position;
    }

    camera.transform().position =
        target_pos - (m_target_distance * camera.get_front())
        + cam_shake;
}

void CameraController::on_late_update(Scene & scene, float delta_time) {
    Input & input = scene.get_input();

    m_pitch = -glm::asin(1.0f / glm::sqrt(3.0f));

    int x, y;
    input.get_cursor_delta(x, y);
    float dx = (float)x * m_mouse_sensitivity;
    m_yaw -= dx;

    orient_camera(scene);

    glm::vec3 pos =  scene.get_camera().get_position();
    Transform tform;
    tform.position = pos;
    get_node(scene).set_global_transform(scene, tform);

    m_screen_shake_elapsed += delta_time;
}

void CameraController::screen_shake(float magnitude,
                    float time,
                    float rampup,
                    float rampdown) {
    m_screen_shake_magnitude = magnitude;
    m_screen_shake_time = time;
    m_screen_shake_rampup = rampup;
    m_screen_shake_rampdown = rampdown;

    m_screen_shake_elapsed = 0.0f;
}
