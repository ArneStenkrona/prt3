#ifndef PRT3_ANIMATION_SYSTEM_H
#define PRT3_ANIMATION_SYSTEM_H

#include "src/engine/animation/animation.h"
#include "src/engine/rendering/model_manager.h"

#include <vector>

namespace prt3 {

class AnimationSystem {
public:
    AnimationSystem(ModelManager & model_manager);

    AnimationID add_animation(
        ModelHandle model_handle,
        char const * animation_name
    );

    void remove_animation(AnimationID id);

    void update(float delta_time);

private:
    ModelManager & m_model_manager;

    std::vector<Animation> m_animations;
    std::vector<AnimationID> m_free_list;
};

} // namespace prt3

#endif
