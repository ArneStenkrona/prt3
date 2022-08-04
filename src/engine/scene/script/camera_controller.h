#ifndef PRT3_CAMERA_CONTROLLER_H
#define PRT3_CAMERA_CONTROLLER_H

#include "src/engine/scene/script/script.h"
#include "src/engine/scene/scene.h"

#include <iostream>

namespace prt3 {

class CameraController : public Script {
public:
    explicit CameraController(Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    virtual void on_init() {
    }
    virtual void on_update() {
        Camera const & camera = scene().get_camera();
        glm::vec3 pos = camera.get_position();
        Transform tform;
        tform.position = pos;
        node().set_global_transform(tform);
    }
private:
};

} // namespace prt3

#endif
