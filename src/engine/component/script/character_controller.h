#ifndef PRT3_CHARACTER_CONTROLLER_H
#define PRT3_CHARACTER_CONTROLLER_H

#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"

#include "src/engine/rendering/model.h"
#include "src/engine/geometry/shapes.h"

#include "src/engine/physics/collider.h"
#include "src/engine/physics/gjk.h"

#include <utility>
#include <iostream>

namespace prt3 {

class CharacterController : public Script {
public:
    enum State {
        IDLE,
        WALK,
        RUN,
        ATTACK_1,
        ATTACK_2,
        JUMP,
        FALL,
        LAND,
        TOTAL_NUM_STATES
    };

    struct StateData {
        float transition;
        Animation::Clip clip_a;
        Animation::Clip clip_b;
        bool can_change_direction;
        bool inherit_animation_time;
    };

    struct CharacterInput {
        glm::vec3 direction;
        glm::vec3 last_grounded_direction;
        bool last_grounded_run;
        bool run;
        bool attack;
        bool jump;
    };

    explicit CharacterController(Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    explicit CharacterController(std::istream &, Scene & scene, NodeID m_node_id)
        : Script(scene, m_node_id) {}

    virtual void on_init(Scene & scene) {
        add_tag(scene, "player");
        m_direction = get_node(scene).local_transform().get_front();

        Armature & armature =
            scene.get_component<Armature>(node_id());
        Model const & model =
            scene.get_model(armature.model_handle());

        m_state_data[IDLE] = {
            .transition = 0.0f,
            .clip_a = { .animation_index = model.get_animation_index("idle"),
                        .paused = false,
                        .looping = true, },
            .clip_b = { .animation_index = NO_ANIMATION },
            .can_change_direction = false,
            .inherit_animation_time = true
        };

        m_state_data[WALK] = {
            .transition = 0.0f,
            .clip_a = { .animation_index = model.get_animation_index("walk"),
                        .paused = false,
                        .looping = true },
            .clip_b = { .animation_index = model.get_animation_index("idle"),
                        .paused = false,
                        .looping = true },
            .can_change_direction = true,
            .inherit_animation_time = true
        };

        m_state_data[RUN] = {
            .transition = 0.0f,
            .clip_a = { .animation_index = model.get_animation_index("run"),
                        .paused = false,
                        .looping = true },
            .clip_b = { .animation_index = model.get_animation_index("walk"),
                        .paused = false,
                        .looping = true },
            .can_change_direction = true,
            .inherit_animation_time = true
        };

        m_state_data[ATTACK_1] = {
            .transition = 0.05f,
            .clip_a = { .animation_index = model.get_animation_index("attack1"),
                        .paused = false,
                        .looping = false },
            .clip_b = { .animation_index = NO_ANIMATION},
            .can_change_direction = false,
            .inherit_animation_time = false
        };

        m_state_data[ATTACK_2] = {
            .transition = 0.05f,
            .clip_a = { .animation_index = model.get_animation_index("attack2"),
                        .paused = false,
                        .looping = false },
            .clip_b = { .animation_index = NO_ANIMATION},
            .can_change_direction = false,
            .inherit_animation_time = false
        };

        m_state_data[JUMP] = {
            .transition = 0.02f,
            .clip_a = { .animation_index = model.get_animation_index("jump"),
                        .speed = 1.25f,
                        .paused = false,
                        .looping = false },
            .clip_b = { .animation_index = NO_ANIMATION},
            .can_change_direction = false,
            .inherit_animation_time = false
        };

        m_state_data[FALL] = {
            .transition = 0.02f,
            .clip_a = { .animation_index = model.get_animation_index("fall"),
                        .paused = false,
                        .looping = true },
            .clip_b = { .animation_index = NO_ANIMATION},
            .can_change_direction = false,
            .inherit_animation_time = false
        };

        m_state_data[LAND] = {
            .transition = 0.02f,
            .clip_a = { .animation_index = model.get_animation_index("land"),
                        .speed = 1.5f,
                        .paused = false,
                        .looping = false },
            .clip_b = { .animation_index = NO_ANIMATION},
            .can_change_direction = false,
            .inherit_animation_time = false
        };

        init_state(scene);
    }

    bool transition_state(Scene & scene, float /*delta_time*/) {
        Input & input = scene.get_input();

        Armature & armature =
            scene.get_component<Armature>(node_id());
        Animation & anim =
            scene.animation_system().get_animation(armature.animation_id());
        Model const & model =
            scene.get_model(armature.model_handle());

        switch (m_state) {
            case IDLE:
            case WALK:
            case RUN: {
                float eps = 0.05f;
                if (m_run_factor <= eps && m_state != IDLE) {
                    m_state = IDLE;
                    return true;
                }
                if (eps < m_run_factor && m_run_factor <= 1.0f && m_state != WALK) {
                    m_state = WALK;
                    return true;
                }
                if (1.0f < m_run_factor && m_state != RUN) {
                    m_state = RUN;
                    return true;
                }

                if (input.get_key_down(KeyCode::KEY_CODE_RETURN)) {
                    m_state = ATTACK_1;
                    return true;
                }
                if (input.get_key_down(KeyCode::KEY_CODE_SPACE)) {
                    m_state = JUMP;
                    return true;
                }
                break;
            }
            case ATTACK_1:
            case ATTACK_2: {
                float a_frac = anim.clip_a.frac(model);
                if (a_frac >= 1.0f) {
                    m_state = IDLE;
                } else if (a_frac >= 0.4f) {
                    if (m_input.direction != glm::vec3{0.0f}) {
                        m_state = IDLE;
                        return true;
                    }
                    if (input.get_key_down(KeyCode::KEY_CODE_SPACE)) {
                        m_state = JUMP;
                        return true;
                    }

                }
                if (input.get_key_down(KeyCode::KEY_CODE_RETURN)) {
                    if (a_frac < 0.75f) {
                        m_state = m_state == ATTACK_1 ? ATTACK_2 : ATTACK_1;
                    }
                    return true;
                }
                break;
            }
            case JUMP: {
                float a_frac = anim.clip_a.frac(model);
                if (a_frac >= 1.0f) {
                    m_state = FALL;
                    return true;
                }
                if (m_grounded && a_frac > 0.5f) {
                    m_state = LAND;
                    return true;
                }
                break;
            }
            case FALL: {
                if (m_grounded) {
                    m_state = LAND;
                    return true;
                }
                break;
            }
            case LAND: {
                float a_frac = anim.clip_a.frac(model);
                if (a_frac >= 1.0f) {
                    m_state = IDLE;
                    return true;
                }
                if (a_frac >= 0.4f) {
                    if (m_input.direction != glm::vec3{0.0f}) {
                        m_state = IDLE;
                        return true;
                    }
                }
                break;
            }
            case TOTAL_NUM_STATES: { assert(false); }
        }
        return false;
    }

    void init_animation(Animation  & animation) {
        StateData const & data = m_state_data[m_state];

        float t_a = data.clip_a.t;
        float t_b = data.clip_b.t;
        if (data.inherit_animation_time) {
            t_a = animation.clip_a.t;
            t_b = animation.clip_b.t;
        }

        if (data.transition > 0.0f) {
            animation.clip_b = animation.clip_a;
        } else {
            animation.clip_b = data.clip_b;
            animation.clip_b.t = t_b;
        }
        animation.clip_a = data.clip_a;
        animation.clip_a.t = t_a;
    }

    void init_state(Scene & scene) {
        if (m_state_data[m_state].transition != 0.0f) {
            m_transition = 0.0f;
            m_transition_complete = false;
        } else {
            m_transition_complete = true;
        }

        Armature & armature =
            scene.get_component<Armature>(node_id());
        Animation & anim =
            scene.animation_system().get_animation(armature.animation_id());
        init_animation(anim);

        switch (m_state) {
            case IDLE: {
                m_run_factor = 0.0f;
                break;
            }
            case JUMP: {
                m_jumped = false;
                break;
            }
            default: {}
        }
    }

    float get_blend_factor() {
        switch (m_state) {
            case IDLE: {
                return 0.0f;
                break;
            }
            case WALK: {
                return glm::clamp(1.0f - (m_run_factor), 0.0f, 1.0f);
                break;
            }
            case RUN: {
                return glm::clamp(1.0f - (m_run_factor - 1.0f), 0.0f, 1.0f);
                break;
            }
            case TOTAL_NUM_STATES: { assert(false); }
            default: {
                return 0.0f;
            }
        }
    }

    void update_animation(Animation & animation) {
        StateData const & data = m_state_data[m_state];

        bool exit_transition = false;
        if (data.transition > 0.0f &&
            m_transition > data.transition &&
            !m_transition_complete) {
            m_transition_complete = true;
            exit_transition = true;
        }

        if (exit_transition) {
            float t_adjusted = data.clip_b.paused ?
                data.clip_b.t :
                (animation.clip_a.t * data.clip_b.speed / animation.clip_a.speed) +
                    data.clip_b.t;
            animation.clip_b = data.clip_b;
            animation.clip_b.t = t_adjusted;
        }

        switch (m_state) {
            case WALK:
            case RUN: {
                float fac = m_state == WALK ?
                    glm::clamp(m_run_factor, 0.0f, 1.0f) :
                    glm::clamp(m_run_factor - 1.0f, 0.0f, 1.0f);
                float low = m_state == WALK ? 1.0 : 1.5;
                float high = m_state == WALK ? 1.5 : 2.5;

                animation.clip_a.speed = glm::mix(low, high, fac);
                animation.clip_b.speed = glm::mix(low, high, fac);
                break;
            }
            default: {}
        }

        animation.blend_factor = !m_transition_complete ?
            glm::clamp(1.0f - m_transition / data.transition, 0.0f, 1.0f) :
            get_blend_factor();
    }

    void update_input(Scene & scene, float /*delta_time*/) {
        Input & input = scene.get_input();

        m_input.run = input.get_key(KeyCode::KEY_CODE_LSHIFT);
        m_input.attack = input.get_key_down(KeyCode::KEY_CODE_RETURN);
        m_input.jump = input.get_key_down(KeyCode::KEY_CODE_SPACE);

        glm::vec3 raw_input_dir{0.0f};

        if (input.get_key(KeyCode::KEY_CODE_W)) {
            raw_input_dir += glm::vec3{1.0f, 0.0f, 0.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_S)) {
            raw_input_dir -= glm::vec3{1.0f, 0.0f, 0.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_A)) {
            raw_input_dir -= glm::vec3{0.0f, 0.0f, 1.0f};
        }
        if (input.get_key(KeyCode::KEY_CODE_D)) {
            raw_input_dir += glm::vec3{0.0f, 0.0f, 1.0f};
        }

        m_input.direction = glm::vec3{ 0.0f };
        // project input according to camera
        if (raw_input_dir != glm::vec3{0.0f}) {
            // compute look direction
            Camera const & camera = scene.get_camera();
            glm::vec3 cf = camera.get_front();
            glm::vec3 cr = camera.get_right();
            m_input.direction = glm::normalize(
                raw_input_dir.x * glm::vec3{cf.x, 0.0f, cf.z} +
                raw_input_dir.z * glm::vec3{cr.x, 0.0f, cr.z} +
                glm::vec3{0.0f, raw_input_dir.y, 0.0f}
            );
        }
        if (m_grounded) {
            m_input.last_grounded_direction = m_input.direction;
            m_input.last_grounded_run = m_input.run;
        }
    }

    void handle_state(Scene & scene, float delta_time) {

        Armature & armature =
            scene.get_component<Armature>(node_id());
        Animation & anim =
            scene.animation_system().get_animation(armature.animation_id());
        Model const & model =
            scene.get_model(armature.model_handle());

        switch (m_state) {
            case IDLE:
            case WALK:
            case RUN: {
                float target_run_factor = 0.0f;
                if (m_input.direction != glm::vec3{0.0f}) {
                    target_run_factor = m_input.run ? 2.0f : 1.0f;
                }

                float dt_fac = 10.0f * delta_time;
                m_run_factor = glm::mix(m_run_factor, target_run_factor, dt_fac);

                float vel_mag = 0.0f;
                if (m_run_factor <= 1.0f) {
                    vel_mag = glm::mix(0.0f, 3.5f, m_run_factor);
                } else {
                    vel_mag = glm::mix(3.5f, 12.0f, m_run_factor - 1.0f);
                }

                m_velocity = m_input.direction * vel_mag;
                break;
            }
            case ATTACK_1:
            case ATTACK_2: {
                float a_frac = anim.clip_a.frac(model);

                float vel_mag = 0.0f;
                if (0.1f < a_frac && a_frac < 0.2f) {
                    vel_mag = m_state == ATTACK_1 ? 16.0f : 18.0f;
                } else {
                    vel_mag = 0.0f;
                }
                m_velocity = m_direction * vel_mag;

                break;
            }
            case JUMP: {
                float a_frac = anim.clip_a.frac(model);
                if (a_frac >= 0.25f && !m_jumped) {
                    m_gravity_velocity = -17.5f;
                    m_jumped = true;
                }

                if (m_grounded) {
                    m_velocity = glm::vec3{0.0f};
                } else {
                    float vel_mag = m_input.last_grounded_run ? 9.0f : 6.0f;
                    m_velocity = m_input.last_grounded_direction * vel_mag;
                }

                break;
            }
            case FALL: {
                break;
            }
            case LAND: {
                m_velocity = glm::vec3{0.0f};
                break;
            }
            case TOTAL_NUM_STATES: { assert(false); }
        }
    }

    void update_direction(Scene & scene, float delta_time) {
        float dt_fac = 10.0f * delta_time;

        if (m_state_data[m_state].can_change_direction &&
            m_input.direction != glm::vec3{0.0f}) {
            m_direction = glm::mix(m_direction, m_input.direction, dt_fac);
        }

        glm::vec3 raw_look_dir = glm::vec3{m_direction.x, 0.0f, m_direction.z};
        if (raw_look_dir != glm::vec3{0.0f}) {
            glm::vec3 look_dir = glm::normalize(raw_look_dir);
            glm::quat rot = glm::rotation(glm::vec3{0.0f, 0.0f, 1.0f}, look_dir);
            get_node(scene).set_global_rotation(scene, rot);
        }
    }

    void update_physics(Scene & scene, float delta_time) {
        glm::vec3 translation = m_velocity * delta_time;
        translation += m_gravity_velocity * delta_time * m_gravity_direction;

        auto res = get_node(scene).move_and_collide(scene, translation);
        m_grounded = res.grounded;
        if (res.grounded) {
            m_gravity_velocity = 0.0f;
            m_gravity_direction = -res.ground_normal;
        } else {
            m_gravity_direction = glm::vec3{0.0f, -1.0f, 0.0f};
        }
        m_gravity_velocity = glm::min(
            m_gravity_velocity + gravity_constant,
            terminal_velocity
        );
    }

    virtual void on_update(Scene & scene, float delta_time) {
        Armature & armature =
            scene.get_component<Armature>(node_id());
        Animation & anim =
            scene.animation_system().get_animation(armature.animation_id());

        update_input(scene, delta_time);

        /* handle transitions */
        if (transition_state(scene, delta_time)) {
            init_state(scene);
        }

        /* update */
        update_animation(anim);
        handle_state(scene, delta_time);
        update_direction(scene, delta_time);
        update_physics(scene, delta_time);

        m_transition += delta_time;
    }

private:
    static constexpr float gravity_constant = 1.5f;
    static constexpr float terminal_velocity = 35.0f;
    float m_gravity_velocity = 0.0f;
    glm::vec3 m_gravity_direction{0.0f, -1.0f, 0.0f};
    float m_run_factor = 0.0f;
    glm::vec3 m_direction = glm::vec3{0.0f};
    glm::vec3 m_velocity = glm::vec3{0.0f};

    State m_state = IDLE;

    float m_transition = 0.0f;
    bool m_transition_complete = false;

    bool m_grounded = true;
    bool m_jumped = false;

    CharacterInput m_input;

    std::array<StateData, TOTAL_NUM_STATES> m_state_data;

REGISTER_SCRIPT(CharacterController, character_controller, 7387722065150816170)
};

} // namespace prt3

#endif
