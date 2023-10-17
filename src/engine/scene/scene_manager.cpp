#include "scene_manager.h"

#include "src/engine/core/context.h"
#include "src/util/mem.h"

#include <sstream>

using namespace prt3;

SceneManager::SceneManager(Context & context)
 : m_context{context} {}

void SceneManager::queue_scene(std::string const & path) {
    m_queued_scene_path = path;
}

TransitionState SceneManager::load_scene_if_queued(
    TransitionState state,
    Scene & scene,
    Scene & edit_scene
) {
    if (!scene_queued()) {
        return NO_TRANSITION;
    }

    if (state == NO_TRANSITION) {
        scene.emit_signal("__scene_exit__", nullptr);
        state = 0;
    }

    if (m_fade_transition && state >= 0) {
        return transition_fade(state, scene);
    }

    if (!m_fade_transition || state == -2) {
        m_fade_exclude_set.clear();

        static std::unordered_set<ModelHandle> existing_models;
        existing_models = scene.referenced_models();

        static std::unordered_set<ResourceID> existing_textures;
        existing_textures = scene.referenced_textures();

        /* load scene */
        std::ifstream in(queued_scene_path(), std::ios::binary);
        scene.deserialize(in);
        in.close();

        /* free unused models */
        for (ModelHandle handle : existing_models) {
            if (scene.referenced_models().find(handle) ==
                scene.referenced_models().end() &&
                edit_scene.referenced_models().find(handle) ==
                edit_scene.referenced_models().end()) {
                m_context.model_manager().free_model(handle);
            }
        }

        for (ResourceID res_id : existing_textures) {
            if (scene.referenced_textures().find(res_id) ==
                scene.referenced_textures().end() &&
                edit_scene.referenced_textures().find(res_id) ==
                edit_scene.referenced_textures().end()) {
                m_context.texture_manager().free_texture_ref(res_id);
            }
        }

        for (Script * script : m_context.project().active_scripts()) {
            scene.internal_add_script(script, true);
        }

        /* start scene */
        scene.start();
        scene.update(1.0f / 60.0f);
    }

    if (m_fade_transition && state != NO_TRANSITION) {
        if ((state = transition_fade(state, scene)) != NO_TRANSITION) {
            return state;
        }
    }
    set_tint(scene, false, glm::vec3{0.0f}, {});

    reset_queue();

    return NO_TRANSITION;
}

TransitionState SceneManager::transition_fade(
    TransitionState state,
    Scene & scene
) {
    uint32_t frame = state >= 0 ?
                     static_cast<uint32_t>(state) :
                     static_cast<uint32_t>(-state - 2);

    float delta_time = 1.0f / 60.0f;
    float time = 0.1f;
    uint32_t n_frames = uint32_t((time / delta_time) + 0.5f);

    float t = float(frame) / float(n_frames - 1);
    if (state >= 0) {
        float sig_data[2] = { t, 1.0f / float(n_frames - 1) };
        scene.emit_signal("__scene_transition_out__", &sig_data[0]);

        t = 1.0f - t;
    }

    glm::vec3 tint = glm::vec3{t};

    set_tint(scene, true, tint, m_fade_exclude_set);

    if (state >= 0) {
        ++state;
    } else {
        --state;
    }

    frame = state >= 0 ?
                     static_cast<uint32_t>(state) :
                     static_cast<uint32_t>(-state - 2);

    if (frame == n_frames) {
        if (state >= 0) {
            return -2;
        } else {
            return NO_TRANSITION;
        }
    }

    return state;
}

void SceneManager::set_tint(
    Scene & scene,
    bool active,
    glm::vec3 tint,
    std::unordered_set<NodeID> const & exclude
) {
    for (auto & comp : scene.get_all_components<MaterialComponent>()) {
        if (exclude.find(comp.node_id()) != exclude.end()) continue;
        comp.material_override().tint_active = active;
        comp.material_override().tint = tint;
    }
    for (auto & comp : scene.get_all_components<ModelComponent>()) {
        if (exclude.find(comp.node_id()) != exclude.end()) continue;
        comp.material_override().tint_active = active;
        comp.material_override().tint = tint;
    }
    for (auto & comp : scene.get_all_components<AnimatedModel>()) {
        if (exclude.find(comp.node_id()) != exclude.end()) continue;
        comp.material_override().tint_active = active;
        comp.material_override().tint = tint;
    }
}
