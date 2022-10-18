#ifndef PRT3_EDITOR_CAMERA_H
#define PRT3_EDITOR_CAMERA_H

#include "src/engine/rendering/camera.h"
#include "src/engine/core/input.h"

namespace prt3 {

class EditorCamera {
public:
    EditorCamera(int width, int height);

    void update(float delta_time, Input const & input);

    Camera & get_camera() { return m_camera; }
private:
    Camera m_camera;

    float m_yaw;
    float m_pitch;
    float m_movement_speed = 25.0f;
    float m_mouse_sensitivity = 0.3f;
};

} // namespace prt3

#endif
