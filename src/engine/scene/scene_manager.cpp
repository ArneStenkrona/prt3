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

        /* save autoload state */
        std::stringstream out_autoload;
        static std::vector<char> data;

        for (UUID uuid : m_context.project().autoload_scripts()) {
            Script const * script = scene.get_autoload_script(uuid);
            if (script != nullptr) {
                write_stream(out_autoload, true);
                script->save_state(scene, out_autoload);
            } else {
                write_stream(out_autoload, false);
            }
        }

        std::string const & s = out_autoload.str();
        data.reserve(s.size());
        data.assign(s.begin(), s.end());

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

        auto const & autoload_scripts = m_context.project().autoload_scripts();

        scene.add_autoload_scripts(autoload_scripts);

        /* restore autoload state */
        imemstream in_autoload(data.data(), data.size());

        for (UUID uuid : autoload_scripts) {
            bool serialized;
            read_stream(in_autoload, serialized);

            if (serialized) {
                Script * script = scene.get_autoload_script(uuid);
                script->restore_state(scene, in_autoload);
            }
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
        scene.emit_signal("scene_transition_out", &sig_data[0]);

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
