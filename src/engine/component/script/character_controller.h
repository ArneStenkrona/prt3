#ifndef PRT3_CHARACTER_CONTROLLER_H
#define PRT3_CHARACTER_CONTROLLER_H

#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"

#include "src/engine/rendering/model.h"
#include "src/engine/geometry/shapes.h"

#include "src/engine/physics/collider.h"
#include "src/engine/physics/gjk.h"

#include <utility>

namespace prt3 {

class CharacterController : public Script {
public:
    explicit CharacterController(Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    virtual void on_init() {
        Model model{"assets/models/moon_island/moon_island.fbx"};

        std::vector<Model::Vertex> v_buf = model.vertex_buffer();
        std::vector<uint32_t> i_buf = model.index_buffer();

        std::vector<Triangle> tris;
        tris.resize(i_buf.size() / 3);

        for (size_t i = 0; i < tris.size(); ++i) {
            size_t ii = 3 * i;
            tris[i].a = v_buf[i_buf[ii]].position;
            tris[i].b = v_buf[i_buf[ii + 1]].position;
            tris[i].c = v_buf[i_buf[ii + 2]].position;
        }

        mesh_collider.set_triangles(std::move(tris));
    }

    virtual void on_update(float delta_time) {
        Camera & camera = scene().get_camera();
        Input & input = scene().get_input();

        float speed = 10.0f * delta_time;

        glm::vec3 raw_input{ 0.0f, 0.0f, 0.0f };
        if (input.get_key(KeyCode::KEY_CODE_W)) {
            raw_input += glm::vec3{1.0f, 0.0f, 0.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_S)) {
            raw_input -= glm::vec3{1.0f, 0.0f, 0.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_A)) {
            raw_input -= glm::vec3{0.0f, 0.0f, 1.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_D)) {
            raw_input += glm::vec3{0.0f, 0.0f, 1.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_Q)) {
            raw_input += glm::vec3{0.0f, 1.0f, 0.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_E)) {
            raw_input -= glm::vec3{0.0f, 1.0f, 0.0f};
        }
        glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
        // project input according to camera
        if (glm::length2(raw_input) > 0.0f) {
            // compute look direction
            glm::vec3 cf = camera.get_front();
            glm::vec3 cr = camera.get_right();
            translation =
                speed *
                glm::normalize(raw_input.x * glm::vec3{cf.x, 0.0f, cf.z} +
                               raw_input.z * glm::vec3{cr.x, 0.0f, cr.z} +
                               glm::vec3{0.0f, raw_input.y, 0.0f});
        }
        get_node().move_and_collide(translation);
    }
private:
    MeshCollider mesh_collider;
};

} // namespace prt3

#endif
