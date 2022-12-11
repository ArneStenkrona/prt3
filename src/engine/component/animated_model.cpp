#include "animated_model.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"

using namespace prt3;

#include "model.h"



AnimatedModel::AnimatedModel(Scene &, NodeID node_id)
 : m_node_id{node_id} {
    m_model_handle = NO_MODEL;
}

AnimatedModel::AnimatedModel(
    Scene &,
    NodeID node_id,
    ModelHandle model_handle
)
 : m_node_id{node_id},
   m_model_handle{model_handle}
{}

AnimatedModel::AnimatedModel(
    Scene & scene,
    NodeID node_id,
    std::istream & in
)
 : m_node_id{node_id}
{
    ModelManager & man = scene.model_manager();

    size_t n_path;
    read_stream(in, n_path);

    static std::string path;
    path.resize(n_path);

    in.read(path.data(), path.size());

    m_model_handle = man.upload_model(path);
}

void AnimatedModel::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    ModelManager const & man = scene.model_manager();
    Model const & model = man.get_model(m_model_handle);

    std::string const & path = model.path();

    write_stream(out, path.size());
    out.write(path.data(), path.size());
}
