#include "dummy_renderer.h"

using namespace prt3;

void DummyRenderer::upload_model(
    ModelHandle,
    Model const & model,
    std::vector<ResourceID> & mesh_resource_ids
) {
    mesh_resource_ids.resize(model.meshes().size());
    for (size_t i = 0; i < model.meshes().size(); ++i) {
        mesh_resource_ids[i] = m_mesh_counter;
        ++m_mesh_counter;
    }
}

ResourceID DummyRenderer::upload_pos_mesh(
    glm::vec3 const *,
    size_t
) {
    ResourceID id = m_mesh_counter;
    ++m_mesh_counter;
    return id;
}

NodeID DummyRenderer::get_selected(int, int) {
    return NO_NODE;
}

ResourceID DummyRenderer::upload_material(Material const & material) {
    ResourceID id = m_materials.size();
    m_materials[id] = material;
    return id;
}

void DummyRenderer::free_material(ResourceID id) {
    m_materials.erase(id);
}

Material const & DummyRenderer::get_material(ResourceID id) const {
    return m_materials.at(id);
}

Material & DummyRenderer::get_material(ResourceID id) {
    return m_materials.at(id);
}

ResourceID DummyRenderer::upload_texture(TextureData const & data) {
    ResourceID id = m_texture_counter;
    m_texture_metadata[id].width = data.width;
    m_texture_metadata[id].height = data.height;
    m_texture_metadata[id].channels = data.channels;
    ++m_texture_counter;
    return id;
}

void DummyRenderer::get_texture_metadata(
    ResourceID id,
    unsigned int & width,
    unsigned int & height,
    unsigned int & channels
) const {
    TextureMetadata md = m_texture_metadata.at(id);
    width = md.width;
    height = md.height;
    channels = md.channels;
}
