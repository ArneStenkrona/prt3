#include "camera.h"

#include <cassert>

using namespace prt3;

Camera::Camera(Input & input,
              glm::vec3 position,
              glm::vec3 up,
              float yaw,
              float pitch)
    : m_position(position),
      m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
      m_world_up(up),
      m_yaw(yaw),
      m_pitch(pitch),
      m_movement_speed(25.0f),
      m_mouse_sensitivity(0.30f),
      m_field_of_view(45.0f),
      m_width(800.0f),
      m_height(600.0f),
      m_near_plane(0.3f),
      m_far_plane(100.0f),
      m_target_distance(5.0f),
      m_input(input) {
    update_camera_vectors();
}

void Camera::set_projection(float width, float height, float near, float far) {
    assert(width > 0 && height > 0);
    m_width = width;
    m_height = height;
    m_near_plane = near;
    m_far_plane = far;
}

glm::mat4 Camera::get_projection_matrix() const {
    return get_projection_matrix(m_near_plane, m_far_plane);
}

glm::mat4 Camera::get_projection_matrix(float near, float far) const {
    return glm::perspective(glm::radians(m_field_of_view), m_width / m_height, near, far);
}

void Camera::get_camera_corners(glm::vec3 & topleft,
                                glm::vec3 & topright,
                                glm::vec3 & bottomleft,
                                glm::vec3 & bottomright) {
    glm::mat4 invmvp = glm::inverse(get_projection_matrix() * get_view_matrix());
    glm::vec4 tl     = invmvp * glm::vec4{ -1.0f, -1.0f, -1.0f, 1.0f };
    glm::vec4 tr    = invmvp * glm::vec4{ 1.0f, -1.0f, -1.0f, 1.0f };
    glm::vec4 bl  = invmvp * glm::vec4{ -1.0f, 1.0f, -1.0f, 1.0f };
    glm::vec4 br = invmvp * glm::vec4{ 1.0f, 1.0f, -1.0f, 1.0f };
    topleft = tl / tl.w;
    topright = tr / tl.w;
    bottomleft = bl / tl.w;
    bottomright = br / tl.w;
}

void Camera::rotate(float delta_yaw, float delta_pitch) {
    m_yaw += delta_yaw;
    m_pitch += delta_pitch;
}

void Camera::set_target(glm::vec3 target) {
    m_position = target - (m_target_distance * m_front);
    m_last_target_position = target;
}

void Camera::update(float delta_time, bool keyboard_movement, bool drag) {
    if (keyboard_movement) {
        process_keyboard(delta_time);
    }
    process_mouse_movement(drag);
}

void Camera::process_keyboard(float delta_time) {
    //Movement
    float camera_speed = m_movement_speed * delta_time;
    if (m_input.get_key(KeyCode::KEY_CODE_W))
        m_position += m_front * camera_speed;
    if (m_input.get_key(KeyCode::KEY_CODE_S))
        m_position -= m_front * camera_speed;
    if (m_input.get_key(KeyCode::KEY_CODE_A))
        m_position -= glm::normalize(glm::cross(m_front, m_up)) * camera_speed;
    if (m_input.get_key(KeyCode::KEY_CODE_D))
        m_position += glm::normalize(glm::cross(m_front, m_up)) * camera_speed;
    if (m_input.get_key(KeyCode::KEY_CODE_SPACE))
        m_position += glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)) * camera_speed;
    if (m_input.get_key(KeyCode::KEY_CODE_LCTRL))
        m_position -= glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)) * camera_speed;
    //Reset field of view
    if (m_input.get_key(KeyCode::KEY_CODE_R))
        m_field_of_view = 45.0f;
}

void Camera::process_mouse_movement(bool drag) {
    if (drag && !m_input.get_key(KeyCode::KEY_CODE_MOUSE_LEFT)) return;

    //Mouse
    int x, y;
    m_input.get_cursor_delta(x, y);
    float dx = (float)x * m_mouse_sensitivity, dy = (float)y * m_mouse_sensitivity;

    m_yaw += dx;
    m_pitch -= dy;

    // constrainPitch should have its effect here
    if (m_pitch > 89.0f)
        m_pitch = 89.0f;
    if (m_pitch < -89.0f)
        m_pitch = -89.0f;

    glm::vec3 f;
    f.x = cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
    f.y = sin(glm::radians(m_pitch));
    f.z = cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));

    m_front = glm::normalize(f);
    // Update Front, Right and Up Vectors using the updated Euler angles
    update_camera_vectors();
}

void Camera::update_camera_vectors() {
    // Calculate the new Front vector
    glm::vec3 f;
    f.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    f.y = sin(glm::radians(m_pitch));
    f.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    m_front = glm::normalize(f);
    // Also re-calculate the Right and Up vector
    m_right = glm::normalize(glm::cross(m_front, m_world_up));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    m_up = glm::normalize(glm::cross(m_right, m_front));
}
