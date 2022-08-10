#ifndef PRT3_CAMERA_H
#define PRT3_CAMERA_H

#include "src/engine/core/input.h"
#include "src/engine/scene/transform.h"

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
           int height);
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

    inline glm::mat4 get_view_matrix() const
        { return glm::lookAt(m_transform.position,
                             m_transform.position + m_transform.get_front(),
                             WORLD_UP); }
    void set_projection(float width, float height, float near, float far);
    glm::mat4 get_projection_matrix() const;
    glm::mat4 get_projection_matrix(float near, float far) const;

    inline float get_field_of_view() const { return m_field_of_view; }

    inline Transform & transform() { return m_transform; }
    inline glm::vec3 const & get_position() const { return m_transform.position; }
    inline glm::vec3 get_front() const { return m_transform.get_front(); }
    inline glm::vec3 get_up() const { return m_transform.get_up(); }
    inline glm::vec3 get_right() const { return m_transform.get_right(); }

    inline float near_plane() const {return m_near_plane; }
    inline float far_plane() const {return m_far_plane; }

    static constexpr glm::vec3 WORLD_UP{ 0.0f, 1.0f, 0.0f };

private:
    Transform m_transform;
    float m_field_of_view;
    float m_width;
    float m_height;
    float m_near_plane;
    float m_far_plane;

    Input & m_input;

    void set_size(int w, int h)
        { m_width = static_cast<float>(w); m_height = static_cast<float>(h); }

    friend class Scene;
};

} // namespace prt3

#endif
