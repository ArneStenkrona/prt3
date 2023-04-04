#include "animated_model.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"

using namespace prt3;

#include "model.h"

AnimatedModel::AnimatedModel(Scene &, NodeID node_id)
 : m_node_id{node_id} {
}

AnimatedModel::AnimatedModel(
    Scene & scene,
    NodeID node_id,
    ModelHandle model_handle
)
 : m_node_id{node_id},
   m_model_handle{model_handle}
{
    if (m_model_handle != NO_MODEL) {
        m_animation_id =
            scene.animation_system().add_animation(scene, model_handle);
    }
}

AnimatedModel::AnimatedModel(
    Scene & scene,
    NodeID node_id,
    std::istream & in
)
 : m_node_id{node_id}
{
    size_t n_path;
    read_stream(in, n_path);

    if (n_path > 0) {
        static std::string path;
        path.resize(n_path);

        in.read(path.data(), path.size());

        m_model_handle = scene.upload_model(path);
    }

    if (m_model_handle != NO_MODEL) {
        m_animation_id =
            scene.animation_system().add_animation(scene, m_model_handle);
    }
}

void AnimatedModel::set_model_handle(
    Scene & scene,
    ModelHandle handle
) {
    if (handle == m_model_handle) return;

    m_model_handle = handle;
    if (handle != NO_MODEL) {
        if (m_animation_id == NO_ANIMATION) {
            m_animation_id =
                scene.animation_system().add_animation(scene, handle);
        } else {
            scene.animation_system().init_animation(scene, handle, m_animation_id);
        }
    } else {
        scene.animation_system().remove_animation(m_animation_id);
        m_animation_id = NO_ANIMATION;
    }
}

void AnimatedModel::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    if (m_model_handle != NO_MODEL) {
        ModelManager const & man = scene.model_manager();

        Model const & model = man.get_model(m_model_handle);

        std::string const & path = model.path();

        write_stream(out, path.size());
        out.write(path.data(), path.size());
    } else {
        write_stream(out, 0);
    }
}

void AnimatedModel::remove(Scene & scene) {
    if (m_animation_id != NO_ANIMATION) {
        scene.animation_system().remove_animation(m_animation_id);
    }
}
