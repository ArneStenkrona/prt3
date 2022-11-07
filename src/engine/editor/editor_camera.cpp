#include "editor_camera.h"

#include <glm/gtx/euler_angles.hpp>

using namespace prt3;

EditorCamera::EditorCamera(int width, int height)
 : m_camera{width, height} {

}

void EditorCamera::update(float delta_time,
                          Input const & input) {
    Transform & cam_tform = m_camera.transform();

    if (input.get_key(KeyCode::KEY_CODE_MOUSE_LEFT)) {
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

    float camera_speed = m_movement_speed * delta_time;

    glm::vec3 front = m_camera.get_front();
    glm::vec3 right = m_camera.get_right();
    if (input.get_key(KeyCode::KEY_CODE_W))
        cam_tform.position += front * camera_speed;
    if (input.get_key(KeyCode::KEY_CODE_S))
        cam_tform.position -= front * camera_speed;
    if (input.get_key(KeyCode::KEY_CODE_A))
        cam_tform.position -= right * camera_speed;
    if (input.get_key(KeyCode::KEY_CODE_D))
        cam_tform.position += right * camera_speed;
    if (input.get_key(KeyCode::KEY_CODE_E))
        cam_tform.position += Camera::WORLD_UP * camera_speed;
    if (input.get_key(KeyCode::KEY_CODE_Q))
        cam_tform.position -= Camera::WORLD_UP * camera_speed;
}
