#include "animated_mesh.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"

using namespace prt3;

AnimatedMesh::AnimatedMesh(Scene &, NodeID node_id)
 : m_node_id{node_id},
   m_resource_id{NO_RESOURCE} {}

AnimatedMesh::AnimatedMesh(
    Scene &,
    NodeID node_id,
    ResourceID resource_id,
    NodeID armature_id
)
 : m_node_id{node_id},
   m_resource_id{resource_id},
   m_armature_id{armature_id} {}

AnimatedMesh::AnimatedMesh(Scene & scene, NodeID node_id, std::istream & in)
 : m_node_id{node_id},
   m_resource_id{NO_RESOURCE} {
    ModelManager & man = scene.model_manager();

    size_t n_path;
    read_stream(in, n_path);

    if (n_path == 0) {
        return;
    }

    static std::string path;
    path.resize(n_path);

    in.read(path.data(), path.size());

    int32_t mesh_index;
    read_stream(in, mesh_index);

    auto handle = scene.upload_model(path);
    m_resource_id = man.get_mesh_id_from_mesh_index(
        handle,
        mesh_index
    );

    thread_local std::vector<int32_t> relative_path;

    read_stream(in, n_path);
    relative_path.resize(n_path);
    for (int32_t & i : relative_path) {
        read_stream(in, i);
    }

    m_armature_id =
        scene.get_node_id_from_relative_path(m_node_id, relative_path);
}

void AnimatedMesh::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    ModelManager const & man = scene.model_manager();
    ResourceID id = m_resource_id;

    if (id != NO_RESOURCE) {
        Model const & model = man.get_model_from_mesh_id(id);

        std::string const & path = model.path();

        write_stream(out, path.size());
        out.write(path.data(), path.size());

        write_stream(out, man.get_mesh_index_from_mesh_id(id));

        thread_local std::vector<int32_t> relative_path;
        scene.find_relative_path(m_node_id, m_armature_id, relative_path);

        write_stream(out, relative_path.size());
        for (int32_t i : relative_path) {
            write_stream(out, i);
        }
    } else {
        write_stream(out, size_t{0});
    }
}
