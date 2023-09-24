#ifndef PRT3_ANIMATION_SYSTEM_H
#define PRT3_ANIMATION_SYSTEM_H

#include "src/engine/animation/animation.h"
#include "src/engine/rendering/model_manager.h"

#include <vector>

namespace prt3 {

class Scene;
class AnimatedModel;
class Armature;

class AnimationSystem {
public:
    AnimationID add_animation(
        Scene const & scene,
        ModelHandle model_handle
    );

    void remove_animation(AnimationID id);

    Animation const & get_animation(AnimationID id) const
    { return m_animations[id]; }
    Animation & get_animation(AnimationID id) { return m_animations[id]; }

    std::vector<Animation> const & animations() const { return m_animations; }

private:
    std::vector<Animation> m_animations;
    std::vector<AnimationID> m_free_list;

    void init_animation(
        Scene const & scene,
        ModelHandle model_handle,
        AnimationID id
    );

    void update(Scene const & scene, float delta_time);
    void clear();

    friend class Scene;
    friend class AnimatedModel;
    friend class Armature;
};

} // namespace prt3

#endif
