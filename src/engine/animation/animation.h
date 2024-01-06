#ifndef PRT3_ANIMATION_H
#define PRT3_ANIMATION_H

#include "src/engine/rendering/model_manager.h"
#include "src/engine/component/transform.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>

namespace prt3 {

typedef int32_t AnimationID;
constexpr int32_t NO_ANIMATION = -1;

class AnimationSystem;
class Scene;

struct Animation {
    struct Clip {
        float t = 0.0f;
        float speed = 1.0f;
        bool paused = false;
        bool looping = false;

        void clear_animation() {
            duration = 0.0f;
            animation_index = -1;
            t_prev = std::numeric_limits<float>::quiet_NaN();
        }

        int32_t get_animation_index() const
        { return animation_index; }

        void set_animation_index(Model const & model, int32_t index) {
            animation_index = index;
            Model::Animation const & animation = model.animations()[index];
            duration = animation.duration / animation.ticks_per_second;
            t_prev = std::numeric_limits<float>::quiet_NaN();
        }

        float frac() const {
            if (animation_index == -1) return 0.0f;
            return t / duration;
        }

        void set_frac(float frac) {
            t = frac * duration;
        }

        float frac_to_timepoint(float frac) const {
            return frac * duration;
        }

        bool passed_timepoint(float tp) const {
            if (t_prev > t)  {
                /* this means that we've looped */
                return t_prev <= tp || tp < t;
            }

            return t_prev <= tp && tp < t;
        }

    private:
        int32_t animation_index = -1;
        float duration = 0.0f;
        float t_prev;

        friend class AnimationSystem;
    };

    ModelHandle model_handle;

    Clip clip_a;
    Clip clip_b;
    float blend_factor;

private:
    std::vector<glm::mat4> transforms;
    std::vector<Transform> local_transforms;

    friend class AnimationSystem;
    friend class Armature;
    friend class Scene;
};

} // namespace prt3

#endif
