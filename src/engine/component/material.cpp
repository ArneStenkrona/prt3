#include "material.h"

#include "src/engine/scene/scene.h"
#include "src/engine/rendering/model_manager.h"

using namespace prt3;

MaterialComponent::MaterialComponent(Scene &, NodeID node_id)
 : m_node_id{node_id},
   m_resource_id{NO_RESOURCE} {}

MaterialComponent::MaterialComponent(
    Scene &,
    NodeID node_id,
    ResourceID resource_id
)
 : m_node_id{node_id},
   m_resource_id{resource_id} {}

MaterialComponent::MaterialComponent(
    Scene & scene,
    NodeID node_id,
    std::istream & in
)
 : m_node_id{node_id} {
    ModelManager & man = scene.model_manager();

    size_t n_path;

    read_stream(in, n_path);

    static std::string path;
    path.resize(n_path);

    in.read(path.data(), path.size());


    int32_t mesh_index;
    read_stream(in, mesh_index);

    auto handle = scene.upload_model(path);
    m_resource_id = man.get_material_id_from_mesh_index(
        handle,
        mesh_index
    );
}

void MaterialComponent::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    ModelManager const & man = scene.model_manager();
    ResourceID id = m_resource_id;

    Model const & model = man.get_model_from_material_id(id);

    std::string const & path = model.path();

    write_stream(out, path.size());
    out.write(path.data(), path.size());

    write_stream(out, man.get_mesh_index_from_material_id(id));
}
