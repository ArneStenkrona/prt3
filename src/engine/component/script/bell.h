#ifndef PRT3_BELL_H
#define PRT3_BELL_H

#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"
#include "src/util/math_util.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <iostream>
#include <cmath>

namespace prt3 {

class Bell : public Script {
public:
    explicit Bell(Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    explicit Bell(std::istream &, Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    virtual void on_late_init(Scene & scene) {
        m_theta = 0.0f;
        m_phi = 0.0f;

        m_d_theta = 0.0f;
        m_d_phi = 0.0f;

        m_hit_timer = m_hit_cooldown;

        m_child_id = scene.get_node(node_id()).children_ids().front();
    }

    virtual void on_update(Scene & scene, float delta_time) {
        float dd_theta =
            (g / m_l) * glm::sin(m_theta) -
            m_d_phi * m_d_phi * glm::cos(m_theta) * glm::sin(m_theta);

        m_d_theta += dd_theta * delta_time;
        m_theta += m_d_theta * delta_time;
        m_phi += m_d_phi * delta_time;

        m_d_theta *= 1.0f - m_dampening * delta_time;
        m_d_phi *= 1.0f - m_dampening * delta_time;

        glm::quat rot = glm::quat_cast(
            glm::yawPitchRoll(m_phi, m_theta, 0.0f)
        );

        get_node(scene).local_transform().rotation = rot;

        std::vector<NodeID> const & overlaps = scene.get_overlaps(m_child_id);

        if (m_hit_timer > m_hit_cooldown && !overlaps.empty())
        {
            NodeID other_id = overlaps.front();

            Node const & other = scene.get_node(other_id);

            glm::vec3 dir =
                other.get_global_transform(scene).position -
                get_node(scene).get_global_transform(scene).position;

            if (dir != glm::vec3{0.0f}) {
                dir = glm::normalize(dir);
            } else {
                dir = glm::vec3{0.0f, 1.0f, 0.0f};
            }

            float phi_dir = glm::pi<float>() / 2.0f - glm::atan(dir.z, dir.x);
            phi_dir = phi_dir - m_phi;

            float theta_dir =
                glm::atan(glm::sqrt(dir.x * dir.x + dir.z * dir.z), dir.y);
            theta_dir = theta_dir - m_theta;

            if (glm::abs(phi_dir) > glm::pi<float>() / 2.0f) {
                phi_dir = phi_dir - glm::sign(phi_dir) * glm::pi<float>();
                theta_dir = -theta_dir;
            }

            float magnitude = 1.0f;

            m_d_theta = theta_dir * magnitude;
            m_d_phi = phi_dir * magnitude;

            m_hit_timer = 0.0f;
        }

        m_theta = wrap_min_max(m_theta, -glm::pi<float>(), glm::pi<float>());
        m_phi = wrap_min_max(m_phi, -glm::pi<float>(), glm::pi<float>());

        m_theta = glm::clamp(m_theta, -m_clamp_theta, m_clamp_theta);

        m_hit_timer += delta_time;
    }

private:
    float m_l = 0.5f;
    float m_theta;
    float m_phi;

    float m_clamp_theta = 0.75f * glm::pi<float>();

    float m_d_theta;
    float m_d_phi;

    float m_dampening = 0.9f;

    float m_hit_timer;
    float m_hit_cooldown = 0.5f;

    static constexpr float g = -9.8f;

    NodeID m_child_id;

REGISTER_SCRIPT(Bell, bell, 14192787994163871465)
};

} // namespace prt3

#endif // PRT3_BELL_H