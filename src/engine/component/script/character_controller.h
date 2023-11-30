#ifndef PRT3_CHARACTER_CONTROLLER_H
#define PRT3_CHARACTER_CONTROLLER_H

#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"

#include "src/engine/rendering/model.h"
#include "src/engine/geometry/shapes.h"

#include "src/engine/physics/collider.h"
#include "src/engine/physics/gjk.h"

#include <glm/gtx/string_cast.hpp>

namespace prt3 {

class CharacterController : public Script {
public:
    typedef uint32_t StateType;
    enum State : StateType {
        NONE             = 0,
        IDLE             = 1 << 0,
        WALK             = 1 << 1,
        RUN              = 1 << 2,
        ATTACK_1         = 1 << 3,
        ATTACK_2         = 1 << 4,
        ATTACK_AIR_1     = 1 << 5,
        ATTACK_AIR_2     = 1 << 6,
        JUMP             = 1 << 7,
        FALL             = 1 << 8,
        LAND             = 1 << 9,
        TOTAL_NUM_STATES = 10 // number of states, excluding 'none'
    };
    static constexpr StateType all_states = ~0;

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

    struct CharacterState {
        glm::vec3 gravity_direction{0.0f, -1.0f, 0.0f};
        float gravity_velocity = 0.0f;
        float run_factor = 0.0f;
        glm::vec3 direction = glm::vec3{0.0f};
        glm::vec3 velocity = glm::vec3{0.0f};
        glm::vec3 force = glm::vec3{0.0f};
        float friction = 10.0f;
        float coyote_time = 0.1f;
        State state = IDLE;
        State queued_state = NONE;
        float transition = 0.0f;
        bool transition_complete = false;
        bool grounded = true;
        glm::vec3 ground_normal;
        float grounded_timer = 0.0f;
        bool jumped = false;
        unsigned int jump_count = 0;
        bool run_jump = false;
        glm::vec3 jump_dir;
        CharacterInput input;
    };

    struct SerializedState {
        CharacterState state;
        Animation::Clip clip_a;
        Animation::Clip clip_b;
        float blend_factor;
        glm::quat rotation;
    };

    explicit CharacterController(Scene & scene, NodeID node_id)
        : Script(scene, node_id) {}

    explicit CharacterController(std::istream &, Scene & scene, NodeID node_id)
        : Script(scene, node_id) {}

    virtual void on_init(Scene & scene);

    bool transition_state(Scene & scene, float /*delta_time*/);

    void init_animation(Animation & animation);

    void init_state(Scene & scene);

    float get_blend_factor() const;

    void update_animation(Animation & animation);

    virtual void update_input(Scene & /*scene*/, float /*delta_time*/) {}
    void update_post_input(Scene & scene);

    void handle_state(Scene & scene, float delta_time);

    void update_direction(Scene & scene, float delta_time);

    void update_physics(Scene & scene, float delta_time);

    virtual void on_update(Scene & scene, float delta_time);

    CharacterState & state() { return m_state; }
    CharacterState const & state() const { return m_state; }

    SerializedState serialize_state(Scene const & scene) const;
    void deserialize_state(
        Scene & scene,
        SerializedState const & serialized
    );

protected:
    static constexpr float gravity_constant = 0.8f;
    static constexpr float terminal_velocity = 20.0f;

    float m_walk_force = 22.0f;
    float m_run_force = 90.0f;
    float m_jump_force = 90.0f;

    float m_walk_anim_speed_a = 1.0f;
    float m_walk_anim_speed_b = 1.5f;
    float m_run_anim_speed = 2.5f;

    CharacterState m_state;

    struct UpdateData {
        float expected_move_distance = 0.0f;
        float move_distance = 0.0f;
    } m_update_data;

    NodeID m_weapon_id = NO_NODE;

    std::array<StateData, TOTAL_NUM_STATES> m_state_data;

    inline State bit_index_to_state(StateType bit_index) {
        return static_cast<State>(1 << bit_index);
    }

    inline StateData & get_state_data(State state) {
        switch (state) {
            case IDLE: return m_state_data[0];
            case WALK: return m_state_data[1];
            case RUN: return m_state_data[2];
            case ATTACK_1: return m_state_data[3];
            case ATTACK_2: return m_state_data[4];
            case ATTACK_AIR_1: return m_state_data[5];
            case ATTACK_AIR_2: return m_state_data[6];
            case JUMP: return m_state_data[7];
            case FALL: return m_state_data[8];
            case LAND: return m_state_data[9];
            default: { assert(false); return m_state_data[0]; }
        }
    }

REGISTER_SCRIPT(CharacterController, character_controller, 7387722065150816170)
};

} // namespace prt3

#endif
