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


struct Animation {
    struct Clip {
        int32_t animation_index;
        float t;
        float speed = 1.0f;
        bool paused = false;
        bool looping = false;

        float frac(Model const & model) const {
            Model::Animation const & animation = model.animations()[animation_index];
            float duration = animation.duration / animation.ticks_per_second;
            return t / duration;
        }
    };

    ModelHandle model_handle;

    Clip clip_a;
    Clip clip_b;
    float blend_factor;

    std::vector<glm::mat4> transforms;
    std::vector<Transform> local_transforms;
};

} // namespace prt3

#endif
