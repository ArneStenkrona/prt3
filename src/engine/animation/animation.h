#ifndef PRT3_ANIMATION_H
#define PRT3_ANIMATION_H

#include "src/engine/rendering/model_manager.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>

namespace prt3 {

typedef int32_t AnimationID;

class AnimationSystem;


struct Animation {
    struct Clip {
        int32_t animation_index;
        float t;
        float speed = 1.0f;
        bool paused = false;
        bool looping = false;
    };

    ModelHandle model_handle;

    Clip clip_a;
    Clip clip_b;
    float blend_factor;

    std::vector<glm::mat4> transforms;
};

} // namespace prt3

#endif
