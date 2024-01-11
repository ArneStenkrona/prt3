#include "character_controller.h"

#include "src/daedalus/game_state/game_state.h"

#include "src/util/random.h"

#include <utility>

using namespace dds;

void CharacterController::on_init(prt3::Scene & scene) {
    m_game_state = scene.get_autoload_script<GameState>();

    m_state.input = {};

    m_state.direction =
        get_node(scene).get_global_transform(scene).get_front();

    m_state.ground_normal = glm::vec3{0.0f, 1.0f, 0.0f};

    m_weapon_id = scene.get_child_with_tag(node_id(), "weapon");

    prt3::Armature & armature =
        scene.get_component<prt3::Armature>(node_id());
    prt3::Model const & model =
        scene.get_model(armature.model_handle());

    StateData & idle = get_state_data(IDLE);
    idle.transition = 0.0f;
    idle.clip_a.set_animation_index(model, model.get_animation_index("idle", 0));
    idle.clip_a.paused = false;
    idle.clip_a.looping = true;
    idle.can_change_direction = false;
    idle.inherit_animation_time = true;

    StateData & walk = get_state_data(WALK);
    walk.transition = 0.0f;
    walk.clip_a.set_animation_index(model, model.get_animation_index("walk", 0));
    walk.clip_a.paused = false;
    walk.clip_a.looping = true;
    walk.clip_b.set_animation_index(model, model.get_animation_index("idle", 0));
    walk.clip_b.paused = false;
    walk.clip_b.looping = true;
    walk.can_change_direction = true;
    walk.inherit_animation_time = true;

    StateData & run = get_state_data(RUN);
    run.transition = 0.0f;
    run.clip_a.set_animation_index(model, model.get_animation_index("run", 0));
    run.clip_a.paused = false;
    run.clip_a.looping = true;
    run.clip_b.set_animation_index(model, model.get_animation_index("walk", 0));
    run.clip_b.paused = false;
    run.clip_b.looping = true;
    run.can_change_direction = true;
    run.inherit_animation_time = true;

    StateData & atk1 = get_state_data(ATTACK_1);
    atk1.transition = 0.05f;
    atk1.clip_a.set_animation_index(model, model.get_animation_index("attack1", 0));
    atk1.clip_a.speed = 0.75f;
    atk1.clip_a.paused = false;
    atk1.clip_a.looping = false;
    atk1.can_change_direction = false;
    atk1.inherit_animation_time = false;

    StateData & atk2 = get_state_data(ATTACK_2);
    atk2.transition = 0.05f;
    atk2.clip_a.set_animation_index(model, model.get_animation_index("attack2", 0));
    atk2.clip_a.speed = 0.75f;
    atk2.clip_a.paused = false;
    atk2.clip_a.looping = false;
    atk2.can_change_direction = false;
    atk2.inherit_animation_time = false;

    StateData & atk_air1 = get_state_data(ATTACK_AIR_1);
    atk_air1.transition = 0.05f;
    atk_air1.clip_a.set_animation_index(model, model.get_animation_index("attack_air1", 0));
    atk_air1.clip_a.speed = 0.75f;
    atk_air1.clip_a.paused = false;
    atk_air1.clip_a.looping = false;
    atk_air1.can_change_direction = false;
    atk_air1.inherit_animation_time = false;

    StateData & atk_air2 = get_state_data(ATTACK_AIR_2);
    atk_air2.transition = 0.05f;
    atk_air2.clip_a.set_animation_index(model, model.get_animation_index("attack_air2", 0));
    atk_air2.clip_a.speed = 0.75f;
    atk_air2.clip_a.paused = false;
    atk_air2.clip_a.looping = false;
    atk_air2.can_change_direction = false;
    atk_air2.inherit_animation_time = false;

    StateData & jump = get_state_data(JUMP);
    jump.transition = 0.02f;
    jump.clip_a.set_animation_index(model, model.get_animation_index("jump", 0));
    jump.clip_a.speed = 1.25f;
    jump.clip_a.paused = false;
    jump.clip_a.looping = false;
    jump.can_change_direction = false;
    jump.inherit_animation_time = false;

    StateData & fall = get_state_data(FALL);
    fall.transition = 0.1f;
    fall.clip_a.set_animation_index(model, model.get_animation_index("fall", 0));
    fall.clip_a.paused = false;
    fall.clip_a.looping = true;
    fall.can_change_direction = false;
    fall.inherit_animation_time = false;

    StateData & land = get_state_data(LAND);
    land.transition = 0.02f;
    land.clip_a.set_animation_index(model, model.get_animation_index("land", 0));
    land.clip_a.speed = 1.5f;
    land.clip_a.paused = false;
    land.clip_a.looping = false;
    land.can_change_direction = false;
    land.inherit_animation_time = false;

    StateData & cast = get_state_data(CAST_SPELL);
    cast.transition = 0.05f;
    cast.clip_a.set_animation_index(model, model.get_animation_index("cast", 0));
    cast.clip_a.speed = 2.0f;
    cast.clip_a.paused = false;
    cast.clip_a.looping = false;
    cast.can_change_direction = false;
    cast.inherit_animation_time = false;

    StateData & dead = get_state_data(DEAD);
    dead.transition = 0.05f;
    dead.clip_a.set_animation_index(model, model.get_animation_index("die", 0));
    dead.clip_a.speed = 1.0f;
    dead.clip_a.paused = false;
    dead.clip_a.looping = false;
    dead.can_change_direction = false;
    dead.inherit_animation_time = false;

    if (!m_state.transition_complete) {
        init_state(scene);
    }

    prt3::NodeID hitbox = scene.get_child_with_tag(node_id(), "hitbox");
    if (hitbox != prt3::NO_NODE) {
        prt3::Weapon::connect_hit_signal(scene, *this, hitbox);
    }
}

bool CharacterController::transition_state(
    prt3::Scene & scene,
    float /*delta_time*/
) {
    prt3::Armature & armature =
        scene.get_component<prt3::Armature>(node_id());
    prt3::Animation & anim =
        scene.animation_system().get_animation(armature.animation_id());

    float a_frac = anim.clip_a.frac();

    StateType transition_mask = NONE;
    StateType queueable_mask = NONE;
    switch (m_state.state) {
        case IDLE: {
            transition_mask = WALK | ATTACK_1 | JUMP | CAST_SPELL;
            if (m_state.grounded_timer > m_state.coyote_time) transition_mask |= FALL;
            queueable_mask = transition_mask;
            break;
        }
        case WALK: {
            transition_mask = IDLE | RUN | ATTACK_1 | JUMP | CAST_SPELL;
            if (m_state.grounded_timer > m_state.coyote_time) transition_mask |= FALL;
            queueable_mask = transition_mask;
            break;
        }
        case RUN: {
            transition_mask = IDLE | WALK | ATTACK_1 | JUMP | CAST_SPELL;
            if (m_state.grounded_timer > m_state.coyote_time) transition_mask |= FALL;
            queueable_mask = transition_mask;
            break;
        }
        case ATTACK_1: {
            queueable_mask = ATTACK_2 | JUMP | CAST_SPELL;
            if (a_frac >= 1.0f) {
                transition_mask = IDLE | WALK | ATTACK_2 | CAST_SPELL | JUMP;
                queueable_mask |= IDLE | WALK;
            } else if (a_frac >= 0.467f) {
                transition_mask = ATTACK_2 | JUMP;
            }

            if (m_state.grounded_timer > m_state.coyote_time) {
                transition_mask |= FALL;
                queueable_mask |= FALL;
            }
            break;
        }
        case ATTACK_2: {
            // busy = false;
            queueable_mask = ATTACK_1 | JUMP | CAST_SPELL;
            if (a_frac >= 1.0f) {
                transition_mask = IDLE | WALK | ATTACK_1 | CAST_SPELL | JUMP;
                queueable_mask |= IDLE | WALK;
            } else if (a_frac >= 0.467f) {
                transition_mask = ATTACK_1 | JUMP;
            }

            if (m_state.grounded_timer > m_state.coyote_time) {
                transition_mask |= FALL;
                queueable_mask |= FALL;
            }
            break;
        }
        case ATTACK_AIR_1: {
            queueable_mask = ATTACK_AIR_2 | JUMP;
            if (a_frac >= 1.0f) {
                transition_mask = FALL | ATTACK_AIR_2 | LAND;
                queueable_mask |= FALL | LAND;
            } else if (a_frac >= 0.467f) {
                transition_mask = ATTACK_AIR_2 | LAND;
                queueable_mask |= LAND;
                if (m_state.jump_count < 2) {
                    transition_mask |= JUMP;
                }
            }
            break;
        }
        case ATTACK_AIR_2: {
            queueable_mask = ATTACK_AIR_1 | JUMP;
            if (a_frac >= 1.0f) {
                transition_mask = FALL | ATTACK_AIR_1 | LAND;
                queueable_mask |= FALL | LAND;
            } else if (a_frac >= 0.467f) {
                transition_mask = ATTACK_AIR_1 | LAND;
                queueable_mask |= LAND;
                if (m_state.jump_count < 2) {
                    transition_mask |= JUMP;
                }
            }
            break;
        }
        case JUMP: {
            queueable_mask = ATTACK_AIR_1 | JUMP;
            if (a_frac >= 1.0f) {
                transition_mask = FALL;
                queueable_mask = FALL;
            }

            if (m_state.grounded && a_frac > 0.5f) {
                transition_mask = LAND;
                queueable_mask = LAND;
            }

            if (!m_state.grounded) {
                transition_mask |= ATTACK_AIR_1;
            }

            if (m_state.jump_count < 2 && a_frac > 0.5f) {
                transition_mask |= JUMP;
            }
            break;
        }
        case FALL: {
            transition_mask = LAND | ATTACK_AIR_1;
            queueable_mask = LAND | ATTACK_AIR_1 | JUMP;
            if (m_state.jump_count < 2) {
                transition_mask |= JUMP;
            }
            break;
        }
        case LAND: {
            queueable_mask = JUMP | ATTACK_1 | CAST_SPELL;
            if (a_frac >= 1.0f) {
                transition_mask = IDLE;
                queueable_mask |= IDLE;
            } else if (a_frac >= 0.25f) {
                transition_mask = WALK | JUMP | ATTACK_1 | CAST_SPELL;
                queueable_mask |= WALK;
            }
            break;
        }
        case CAST_SPELL: {
            queueable_mask = ATTACK_1 | JUMP | CAST_SPELL;
            if (a_frac >= 1.0f) {
                transition_mask = IDLE | WALK | ATTACK_1 | CAST_SPELL;
                queueable_mask |= IDLE | WALK;
            }

            if (m_state.grounded_timer > m_state.coyote_time) {
                transition_mask |= FALL;
                queueable_mask |= FALL;
            }
            break;
        case DEAD:
            break;
        }
        default: { assert(false); }
    }

    auto attempt_queue_state =
        [&queueable_mask, this](State state) {
            this->m_state.queued_state =
                state & queueable_mask ? state : this->m_state.queued_state;
        };

    if (m_state.hp <= 0.0f && m_state.state != DEAD) {
        queueable_mask = DEAD;
        transition_mask = DEAD;
        attempt_queue_state(DEAD);
    }

    float eps = 0.05f;
    if (m_state.run_factor <= eps) {
        attempt_queue_state(IDLE);
    }

    if (!m_state.grounded) {
        attempt_queue_state(FALL);
    }

    if ((eps < m_state.run_factor || m_state.input.direction != glm::vec3{0.0f}) &&
        m_state.run_factor <= 1.0f) {
        attempt_queue_state(WALK);
    }

    if (1.0f < m_state.run_factor) {
        attempt_queue_state(RUN);
    }

    if (m_state.input.attack) {
        attempt_queue_state(ATTACK_1);
        attempt_queue_state(ATTACK_2);
        attempt_queue_state(ATTACK_AIR_1);
        attempt_queue_state(ATTACK_AIR_2);
    }

    if (m_state.input.jump) {
        attempt_queue_state(JUMP);
    }

    if (m_state.input.cast_spell) {
        attempt_queue_state(CAST_SPELL);
    }

    if (m_state.grounded) {
        attempt_queue_state(LAND);
    }

    if (transition_mask & m_state.queued_state) {
        m_state.state = m_state.queued_state;
        m_state.queued_state = NONE;
        return true;
    }
    return false;
}

void CharacterController::init_animation(prt3::Animation & animation) {
    StateData const & data = get_state_data(m_state.state);

    float t_a = data.clip_a.t;
    float t_b = data.clip_b.t;
    if (data.inherit_animation_time) {
        t_a = animation.clip_a.t;
        t_b = animation.clip_b.t;
    }

    if (data.transition > 0.0f) {
        animation.clip_b = animation.clip_a;
        animation.clip_b.paused = true;
    } else {
        animation.clip_b = data.clip_b;
        animation.clip_b.t = t_b;
    }
    animation.clip_a = data.clip_a;
    animation.clip_a.t = t_a;
}

void CharacterController::init_state(prt3::Scene & scene) {
    if (get_state_data(m_state.state).transition != 0.0f) {
        m_state.transition = 0.0f;
        m_state.transition_complete = false;
    } else {
        m_state.transition_complete = true;
    }

    prt3::Armature & armature =
        scene.get_component<prt3::Armature>(node_id());
    prt3::Animation & anim =
        scene.animation_system().get_animation(armature.animation_id());
    init_animation(anim);

    switch (m_state.state) {
        case IDLE: {
            m_state.run_factor = 0.0f;
            break;
        }
        case WALK:
        case RUN: {
            break;
        }
        case ATTACK_1:
        case ATTACK_2:{
            m_state.run_factor = 0.0f;
            if (m_state.input.direction != glm::vec3{0.0f}) {
                m_state.direction = m_state.input.direction;
            }
            break;
        }
        case ATTACK_AIR_1:
        case ATTACK_AIR_2: {
            m_state.run_factor = 0.0f;
            if (m_state.input.direction != glm::vec3{0.0f}) {
                m_state.direction = m_state.input.direction;
            }
            break;
        }
        case JUMP: {
            m_state.run_factor = 0.0f;
            m_state.jumped = false;
            if (m_state.jump_count > 0) {
                anim.clip_a.set_frac(0.05f);
            }
            m_state.jump_dir = m_state.jump_count == 0 ?
                m_state.input.last_grounded_direction : m_state.input.direction;
            m_state.run_jump = m_state.jump_count == 0 ? m_state.input.last_grounded_run : false;
            break;
        }
        case FALL: {
            m_state.run_factor = 0.0f;
            m_state.jump_count = m_state.jump_count == 0 ? 1 : m_state.jump_count;
        }
        default: {
            m_state.run_factor = 0.0f;
        }
    }
}

float CharacterController::get_blend_factor() const {
    switch (m_state.state) {
        case IDLE: {
            return 0.0f;
        }
        case WALK: {
            return glm::clamp(1.0f - (m_state.run_factor), 0.0f, 1.0f);
        }
        case RUN: {
            return glm::clamp(1.0f - (m_state.run_factor - 1.0f), 0.0f, 1.0f);
        }
        default: {
            return 0.0f;
        }
    }
}

void CharacterController::update_animation(prt3::Scene & scene) {
    prt3::Armature & armature =
        scene.get_component<prt3::Armature>(node_id());
    prt3::Animation & anim =
        scene.animation_system().get_animation(armature.animation_id());

    StateData const & data = get_state_data(m_state.state);

    bool exit_transition = false;
    if (data.transition > 0.0f &&
        m_state.transition > data.transition &&
        !m_state.transition_complete) {
        m_state.transition_complete = true;
        exit_transition = true;
    }

    if (exit_transition) {
        float t_adjusted = data.clip_b.paused ?
            data.clip_b.t :
            (anim.clip_a.t * data.clip_b.speed / anim.clip_a.speed) +
                data.clip_b.t;
        anim.clip_b = data.clip_b;
        anim.clip_b.t = t_adjusted;
    }

    switch (m_state.state) {
        case WALK:
        case RUN: {
            float fac = m_state.state == WALK ?
                glm::clamp(m_state.run_factor, 0.0f, 1.0f) :
                glm::clamp(m_state.run_factor - 1.0f, 0.0f, 1.0f);
            float low = m_state.state == WALK ?
                m_walk_anim_speed_a : m_walk_anim_speed_b;
            float high = m_state.state == WALK ?
                m_walk_anim_speed_b : m_run_anim_speed;

            anim.clip_a.speed = glm::mix(low, high, fac);
            anim.clip_b.speed = glm::mix(low, high, fac);

            if (m_state.grounded) {
                prt3::Animation::Clip & clip = anim.blend_factor <= 0.5f ?
                    anim.clip_a : anim.clip_b;

                float t_step0 = clip.frac_to_timepoint(0.0f);
                float t_step1 = clip.frac_to_timepoint(0.5f);

                if (clip.passed_timepoint(t_step0) ||
                    clip.passed_timepoint(t_step1)) {
                    m_game_state->sound_pool().play_sound(
                        scene,
                        SoundPool::footstep_default,
                        glm::mix(0.9f, 1.1f, random_float()),
                        glm::mix(0.9f, 1.1f, random_float()),
                        0.75f,
                        100.0f,
                        scene.get_cached_transform(node_id()).position
                    );
                }
            }
            break;
        }
        default: {}
    }

    anim.blend_factor = !m_state.transition_complete ?
        glm::clamp(1.0f - m_state.transition / data.transition, 0.0f, 1.0f) :
        get_blend_factor();
}

void CharacterController::handle_state(prt3::Scene & scene, float delta_time) {
    prt3::Armature & armature =
        scene.get_component<prt3::Armature>(node_id());
    prt3::Animation & anim =
        scene.animation_system().get_animation(armature.animation_id());

    float speed = 20.0f;
    float dt_fac = 1.0f - glm::pow(0.5f, delta_time * speed);

    bool active_weapon = false;

    switch (m_state.state) {
        case IDLE:
        case WALK:
        case RUN: {
            float target_run_factor = 0.0f;
            if (m_state.input.direction != glm::vec3{0.0f}) {
                target_run_factor = m_state.input.run ? 2.0f : 1.0f;
            }

            m_state.run_factor = glm::mix(m_state.run_factor, target_run_factor, dt_fac);

            float force_mag = 0.0f;
            if (m_state.input.direction != glm::vec3{0.0f}) {
                force_mag = m_state.input.run ? m_run_force : m_walk_force;
            }
            m_state.force = m_state.input.direction * force_mag;
            break;
        }
        case ATTACK_1:
        case ATTACK_2: {
            float a_frac = anim.clip_a.frac();
            active_weapon = 0.15f < a_frac && a_frac < 0.3f;
            float force_mag = 0.0f;
            if (0.15f < a_frac && a_frac < 0.2f) {
                force_mag = 128.0f;
            }

            m_state.force = m_state.direction * force_mag;
            break;
        }
        case ATTACK_AIR_1:
        case ATTACK_AIR_2: {
            float force_mag = m_state.grounded ? 0.0f : 45.0f;
            m_state.force = m_state.input.direction * force_mag;

            float a_frac = anim.clip_a.frac();
            active_weapon = 0.1f < a_frac && a_frac < 0.25f;
            break;
        }
        case JUMP: {
            float a_frac = anim.clip_a.frac();
            if (a_frac >= 0.25f && !m_state.jumped) {
                m_state.gravity_velocity = -14.5f;
                m_state.force = m_state.jump_dir * m_jump_force;
                m_state.jumped = true;
                ++m_state.jump_count;
            } else if (a_frac >= 0.25f) {
                m_state.force = m_state.input.direction * 45.0f;
            }
            break;
        }
        case FALL: {
            m_state.force = m_state.input.direction * 45.0f;
            break;
        }
        case LAND: {
            m_state.force = glm::vec3{0.0f};
            break;
        }
        case CAST_SPELL: {
            m_state.force = glm::vec3{0.0f};
            break;
        }
        case DEAD: {
            m_state.force = glm::vec3{0.0f};
            break;
        }
        default: { assert(false); }
    }

    if (m_weapon_id != prt3::NO_NODE) {
        scene.get_component<prt3::Weapon>(m_weapon_id)
                .set_active(scene, active_weapon);
    }
}

void CharacterController::update_direction(prt3::Scene & scene, float delta_time) {
    float speed = 50.0f;
    float dt_fac = 1.0f - glm::pow(0.5f, delta_time * speed);

    if (get_state_data(m_state.state).can_change_direction &&
        m_state.input.direction != glm::vec3{0.0f}) {
        m_state.direction = glm::normalize(
            glm::mix(m_state.direction, m_state.input.direction, dt_fac)
        );
    }

    glm::vec3 raw_look_dir =
        glm::vec3{m_state.direction.x, 0.0f, m_state.direction.z};
    if (raw_look_dir != glm::vec3{0.0f}) {
        glm::vec3 look_dir = glm::normalize(raw_look_dir);
        glm::quat rot = glm::rotation(glm::vec3{0.0f, 0.0f, 1.0f}, look_dir);
        get_node(scene).set_global_rotation(scene, rot);
    }
}

void CharacterController::update_physics(prt3::Scene & scene, float delta_time) {
    // force
    m_state.velocity = m_state.velocity + m_state.force * delta_time;
    // friction
    m_state.velocity = m_state.velocity / (1.0f + (m_state.friction * delta_time));

    glm::vec3 translation = m_state.velocity * delta_time;
    translation += m_state.gravity_velocity * delta_time * m_state.gravity_direction;

    auto res = get_node(scene).move_and_collide(scene, translation);

    m_state.grounded = res.grounded;
    if (res.grounded) {
        m_state.gravity_velocity = 0.0f;
        m_state.ground_normal = res.ground_normal;
        m_state.gravity_direction = -res.ground_normal;
        m_state.friction = 10.0f;
    } else {
        m_state.gravity_direction = glm::vec3{0.0f, -1.0f, 0.0f};
        m_state.friction = 5.0f;
    }

    m_update_data.expected_move_distance = glm::length(translation);
    m_update_data.move_distance = res.move_distance;

    if (m_state.grounded_timer <= m_state.coyote_time && m_state.gravity_velocity > 0.0f) {
        prt3::PhysicsSystem const & sys = scene.physics_system();

        glm::vec3 pos = get_node(scene).get_global_transform(scene).position;
        float snap_dist = 0.05f;

        prt3::RayHit hit;

        prt3::ColliderTag tag =
            scene.get_component<prt3::ColliderComponent>(node_id()).tag();
        prt3::CollisionLayer mask = sys.get_collision_mask(tag);

        if (sys.raycast(
                pos,
                -m_state.ground_normal,
                snap_dist,
                mask,
                tag,
                hit
        )) {
            get_node(scene).set_global_position(scene, hit.position);
        }
    }

    m_state.gravity_velocity = glm::min(
        m_state.gravity_velocity + gravity_constant,
        terminal_velocity
    );

    m_state.grounded_timer = m_state.grounded ? 0.0f : m_state.grounded_timer + delta_time;
}

void CharacterController::update_post_input(prt3::Scene & /*scene*/) {
    if (m_state.grounded) {
        m_state.input.last_grounded_direction = m_state.input.direction;
        m_state.input.last_grounded_run = m_state.input.run;
        m_state.jump_count = 0;
        // project movement onto ground
        float mag = glm::length(m_state.input.direction);
        m_state.input.direction = m_state.input.direction -
                            glm::dot(m_state.input.direction,
                                        m_state.ground_normal) *
                                m_state.ground_normal;
        if (m_state.input.direction != glm::vec3{0.0f}) {
            m_state.input.direction =
                mag * glm::normalize(m_state.input.direction);
        }
    }
}

void CharacterController::on_update(prt3::Scene & scene, float delta_time) {
    update_input(scene, delta_time);
    update_post_input(scene);

    /* handle transitions */
    if (transition_state(scene, delta_time)) {
        init_state(scene);
    }

    /* update */
    update_animation(scene);
    handle_state(scene, delta_time);
    update_direction(scene, delta_time);
    update_physics(scene, delta_time);

    m_state.transition += delta_time;
}

dds::CharacterController::SerializedState CharacterController::serialize_state(
    prt3::Scene const & scene
) const {
    prt3::Armature const & armature =
        scene.get_component<prt3::Armature>(node_id());
    prt3::Animation const & anim =
        scene.animation_system().get_animation(armature.animation_id());

    prt3::Transform global_tform =
        scene.get_node(node_id()).get_global_transform(scene);

    SerializedState serialized;
    serialized.state = m_state;
    serialized.clip_a = anim.clip_a;
    serialized.clip_b = anim.clip_b;
    serialized.blend_factor = anim.blend_factor;
    serialized.rotation = global_tform.rotation;

    return serialized;
}

void CharacterController::deserialize_state(
    prt3::Scene & scene,
    SerializedState const & serialized
) {
    prt3::Armature & armature =
        scene.get_component<prt3::Armature>(node_id());
    prt3::Animation & anim =
        scene.animation_system().get_animation(armature.animation_id());

    m_state = serialized.state;
    anim.clip_a = serialized.clip_a.get_animation_index() == prt3::NO_ANIMATION ?
        get_state_data(m_state.state).clip_a : serialized.clip_a;
    anim.clip_b = serialized.clip_b.get_animation_index() == prt3::NO_ANIMATION ?
        get_state_data(m_state.state).clip_b : serialized.clip_b;
    anim.blend_factor = serialized.blend_factor;

    prt3::Node & node = scene.get_node(node_id());
    node.set_global_rotation(scene, serialized.rotation);
}
