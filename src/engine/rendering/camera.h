#ifndef PRT3_CAMERA_H
#define PRT3_CAMERA_H

#include "src/engine/core/input.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace prt3 {

class Camera {
public:
    // Constructor with vectors
    Camera(Input & input,
           int width,
           int height,
           glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f);
    // Constructor with scalar values
    Camera(Input& input, float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    /**
     * Retrieve world-space coordinates of camera
     * corners
     * @param topleft top left corner, returned by reference
     * @param topleft top right corner, returned by reference
     * @param topleft bottom left corner, returned by reference
     * @param topleft bottom right corner, returned by reference
     */
    void get_camera_corners(glm::vec3 & topleft,
                            glm::vec3 & topright,
                            glm::vec3 & bottomleft,
                            glm::vec3 & bottomright);

    void set_target(glm::vec3 target);
    void rotate(float delta_yaw, float delta_pitch);

    void update(float delta_time, bool keyboard_movement, bool drag);

    inline glm::mat4 get_view_matrix() const
        { return glm::lookAt(m_position, m_position + m_front, m_up); }
    void set_projection(float width, float height, float near, float far);
    glm::mat4 get_projection_matrix() const;
    glm::mat4 get_projection_matrix(float near, float far) const;

    inline float get_field_of_view() const { return m_field_of_view; }

    inline float get_target_distance() const { return m_target_distance; }
    inline void set_target_distance(float distance) { m_target_distance = distance; }

    inline glm::vec3 const & get_position() const { return m_position; }
    inline glm::vec3 const & get_front() const { return m_front; }
    inline glm::vec3 const & get_up() const { return m_up; }
    inline glm::vec3 const & get_right() const { return m_right; }
    inline glm::vec3 const & get_world_up() const { return m_world_up; }

    inline float near_plane() const {return m_near_plane; }
    inline float far_plane() const {return m_far_plane; }

private:
    // Camera Attributes
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_world_up;
    // Euler Angles
    float m_yaw;
    float m_pitch;
    // Camera options
    float m_movement_speed;
    float m_mouse_sensitivity;
    float m_field_of_view;
    // Projection
    float m_width;
    float m_height;
    float m_near_plane;
    float m_far_plane;

    // Target attributes
    float m_target_distance;
    glm::vec3 m_last_target_position;

    Input & m_input;

    void process_keyboard(float delta_time);
    void process_mouse_movement(bool drag);
    void update_camera_vectors();

    void set_size(int w, int h)
        { m_width = static_cast<float>(w); m_height = static_cast<float>(h); }

    friend class Scene;
};

} // namespace prt3

#endif
