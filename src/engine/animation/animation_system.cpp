#include "animation_system.h"

#include "src/engine/rendering/model.h"

using namespace prt3;

AnimationSystem::AnimationSystem(ModelManager & model_manager)
 : m_model_manager{model_manager} {

}

AnimationID AnimationSystem::add_animation(
    ModelHandle model_handle,
    char const * animation_name
) {
    AnimationID id;
    if (m_free_list.empty()) {
        id = m_animations.size();
        m_animations.push_back({});
    } else {
        id = m_free_list.back();
        m_free_list.pop_back();
    }

    Model const & model = m_model_manager.get_model(model_handle);

    m_animations[id].model_handle = model_handle;
    m_animations[id].clip_a.animation_index = model.get_animation_index(animation_name);
    m_animations[id].clip_a.t = 0;
    m_animations[id].clip_a.speed = 1.0f;
    m_animations[id].clip_a.paused = false;
    m_animations[id].clip_a.looping = true;

    m_animations[id].clip_b.animation_index = -1;
    m_animations[id].clip_b.t = 0;
    m_animations[id].clip_b.speed = 1.0f;
    m_animations[id].clip_b.paused = false;
    m_animations[id].clip_b.looping = true;

    m_animations[id].blend_factor = 0.0f;
    m_animations[id].transforms.resize(model.animations().size());

    return id;
}

void AnimationSystem::remove_animation(AnimationID id) {
    m_free_list.push_back(id);

    m_animations[id] = {};
    m_animations[id].model_handle = NO_MODEL;
}

void AnimationSystem::update(float delta_time) {
    std::vector<Model> const & models = m_model_manager.models();

    for (Animation & animation : m_animations) {
        if (animation.model_handle == NO_MODEL) {
            continue;
        }

        Model const & model = models[animation.model_handle];
        if (animation.clip_b.animation_index == -1 ||
            animation.blend_factor == 0.0f) {
            model.sample_animation(
                animation.clip_a.animation_index,
                animation.clip_a.t,
                animation.clip_a.looping,
                animation.transforms.data()
            );
        } else if (animation.blend_factor == 1.0f) {
            model.sample_animation(
                animation.clip_b.animation_index,
                animation.clip_b.t,
                animation.clip_b.looping,
                animation.transforms.data()
            );
        } else {
            model.blend_animation(
                animation.clip_a.animation_index,
                animation.clip_a.t,
                animation.clip_a.looping,
                animation.clip_b.animation_index,
                animation.clip_b.t,
                animation.clip_b.looping,
                animation.blend_factor,
                animation.transforms.data()
            );
        }
    }

    for (Animation & animation : m_animations) {
        if (animation.model_handle == NO_MODEL) {
            continue;
        }

        Model const & model = models[animation.model_handle];

        if (!animation.clip_a.paused) {
            Animation::Clip & clip = animation.clip_a;
            float duration =
                model.animations()[clip.animation_index].duration;
            clip.t += clip.speed * delta_time;
            clip.t = clip.looping ?
                glm::max(clip.t, duration) :
                fmod(clip.t, duration);
        }

        if (!animation.clip_b.paused) {
            Animation::Clip & clip = animation.clip_b;
            float duration =
                model.animations()[clip.animation_index].duration;
            clip.t += clip.speed * delta_time;
            clip.t = clip.looping ?
                glm::max(clip.t, duration) :
                fmod(clip.t, duration);
        }
    }
}
