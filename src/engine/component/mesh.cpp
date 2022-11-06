#include "mesh.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"

using namespace prt3;

Mesh::Mesh(Scene & scene, NodeID node_id, ResourceID resource_id)
 : m_scene{scene},
   m_node_id{node_id},
   m_resource_id{resource_id} {}

Mesh::Mesh(Scene & scene, NodeID node_id, std::istream & in)
 : m_scene{scene},
   m_node_id{node_id} {
    ModelManager & man = m_scene.model_manager();

    size_t n_path;

    read_stream(in, n_path);

    static std::string path;
    path.resize(n_path);

    in.read(path.data(), path.size());


    int32_t mesh_index;
    read_stream(in, mesh_index);

    auto handle = man.upload_model(path);
    m_resource_id = man.get_mesh_id_from_mesh_index(
        handle,
        mesh_index
    );
}

namespace prt3 {

std::ostream & operator << (
    std::ostream & out,
    Mesh const & component
) {
    Scene const & scene = component.scene();
    ModelManager const & man = scene.model_manager();
    ResourceID id = component.resource_id();
    Model const & model = man.get_model_from_mesh_id(id);

    std::string const & path = model.path();

    write_stream(out, path.size());
    out.write(path.data(), path.size());

    write_stream(out, man.get_mesh_index_from_mesh_id(id));

    return out;
}

} // namespace prt3
