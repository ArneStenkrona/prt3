#include "camera.h"

#include <cassert>

using namespace prt3;

Camera::Camera(int width,
               int height)
    : m_field_of_view(45.0f),
      m_width(static_cast<float>(width)),
      m_height(static_cast<float>(height)),
      m_near_plane(0.3f),
      m_far_plane(1000.0f),
      m_orthographic(false) {
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
    if (m_orthographic) {
        float h = 7.5f;
        float w = h * (m_width / m_height);
        return glm::ortho(-w, w, -h, h, near, far);
    }
    return glm::perspective(glm::radians(m_field_of_view), m_width / m_height, near, far);
}

void Camera::get_camera_corners(glm::vec3 & topleft,
                                glm::vec3 & topright,
                                glm::vec3 & bottomleft,
                                glm::vec3 & bottomright) {
    glm::mat4 invmvp = glm::inverse(get_projection_matrix() * get_view_matrix());
    glm::vec4 tl = invmvp * glm::vec4{ -1.0f, -1.0f, -1.0f, 1.0f };
    glm::vec4 tr = invmvp * glm::vec4{ 1.0f, -1.0f, -1.0f, 1.0f };
    glm::vec4 bl = invmvp * glm::vec4{ -1.0f, 1.0f, -1.0f, 1.0f };
    glm::vec4 br = invmvp * glm::vec4{ 1.0f, 1.0f, -1.0f, 1.0f };
    topleft      = tl / tl.w;
    topright     = tr / tl.w;
    bottomleft   = bl / tl.w;
    bottomright  = br / tl.w;
}

void Camera::collect_camera_render_data(CameraRenderData & camera_data) const {
    camera_data.view_matrix = get_view_matrix();
    camera_data.projection_matrix = get_projection_matrix();
    camera_data.view_position = get_position();
    camera_data.view_direction = get_front();
    camera_data.near_plane = near_plane();
    camera_data.far_plane = far_plane();
}
