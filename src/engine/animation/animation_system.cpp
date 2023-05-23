#include "animation_system.h"

#include "src/engine/rendering/model.h"
#include "src/engine/scene/scene.h"

using namespace prt3;

AnimationID AnimationSystem::add_animation(
    Scene const & scene,
    ModelHandle model_handle
) {
    AnimationID id;
    if (m_free_list.empty()) {
        id = m_animations.size();
        m_animations.push_back({});
    } else {
        id = m_free_list.back();
        m_free_list.pop_back();
    }

    init_animation(scene, model_handle, id);

    return id;
}

void AnimationSystem::remove_animation(AnimationID id) {
    m_free_list.push_back(id);

    m_animations[id] = {};
    m_animations[id].model_handle = NO_MODEL;
}

void AnimationSystem::init_animation(
    Scene const & scene,
    ModelHandle model_handle,
    AnimationID id
) {
    Model const & model = scene.model_manager().get_model(model_handle);

    m_animations[id].model_handle = model_handle;
    m_animations[id].clip_a.animation_index = 0;
    m_animations[id].clip_a.t = 0;
    m_animations[id].clip_a.speed = 1.0f;
    m_animations[id].clip_a.paused = false;
    m_animations[id].clip_a.looping = true;

    m_animations[id].clip_b.animation_index = NO_ANIMATION;
    m_animations[id].clip_b.t = 0;
    m_animations[id].clip_b.speed = 1.0f;
    m_animations[id].clip_b.paused = false;
    m_animations[id].clip_b.looping = true;

    m_animations[id].blend_factor = 0.0f;
    m_animations[id].transforms
        .resize(model.bones().size(), glm::mat4{1.0f});
    m_animations[id].local_transforms
        .resize(model.bones().size(), {});
}

void AnimationSystem::update(Scene const & scene, float delta_time) {
    std::vector<Model> const & models = scene.model_manager().models();

    for (Animation & animation : m_animations) {
        if (animation.model_handle == NO_MODEL) {
            continue;
        }

        if (animation.clip_a.animation_index == NO_ANIMATION) {
            continue;
        }

        Model const & model = models[animation.model_handle];
        if (animation.clip_b.animation_index == NO_ANIMATION ||
            animation.blend_factor == 0.0f) {
            model.sample_animation(
                animation.clip_a.animation_index,
                animation.clip_a.t,
                animation.clip_a.looping,
                animation.transforms.data(),
                animation.local_transforms.data()
            );
        } else if (animation.blend_factor == 1.0f) {
            model.sample_animation(
                animation.clip_b.animation_index,
                animation.clip_b.t,
                animation.clip_b.looping,
                animation.transforms.data(),
                animation.local_transforms.data()
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
                animation.transforms.data(),
                animation.local_transforms.data()
            );
        }
    }

    for (Animation & animation : m_animations) {
        if (animation.model_handle == NO_MODEL) {
            continue;
        }

        Model const & model = models[animation.model_handle];

        if (animation.clip_a.animation_index != NO_ANIMATION &&
            !animation.clip_a.paused) {
            Animation::Clip & clip = animation.clip_a;

            Model::Animation const & model_anim = model.animations()[clip.animation_index];
            float duration = model_anim.duration / model_anim.ticks_per_second;

            clip.t += clip.speed * delta_time;
            clip.t = clip.looping ?
                fmod(clip.t, duration) :
                glm::min(clip.t, duration);
        }

        if (animation.clip_b.animation_index != NO_ANIMATION &&
            !animation.clip_b.paused) {
            Animation::Clip & clip = animation.clip_b;

            Model::Animation const & model_anim = model.animations()[clip.animation_index];
            float duration = model_anim.duration / model_anim.ticks_per_second;
            clip.t += clip.speed * delta_time;
            clip.t = clip.looping ?
                fmod(clip.t, duration) :
                glm::min(clip.t, duration);
        }
    }
}

void AnimationSystem::clear() {
    m_animations.clear();
    m_free_list.clear();
}
