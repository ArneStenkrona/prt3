#include "material_manager.h"

#include "src/engine/core/context.h"

using namespace prt3;

MaterialManager::MaterialManager(Context & context)
 : m_context{context}
{}

ResourceID MaterialManager::upload_material(
    Model::MeshMaterial const & mesh_material
) {
    Material material;
    material.name = mesh_material.name;
    material.albedo = mesh_material.albedo;
    material.metallic = mesh_material.metallic;
    material.roughness = mesh_material.roughness;
    material.ao = mesh_material.ao;
    material.emissive = mesh_material.emissive;
    material.twosided = mesh_material.twosided;
    material.transparent = mesh_material.transparent;

    TextureManager & tex_man = m_context.texture_manager();
    material.albedo_map = mesh_material.albedo_map.empty() ?
        NO_RESOURCE : tex_man.upload_texture(mesh_material.albedo_map);
    material.normal_map = mesh_material.normal_map.empty() ?
        NO_RESOURCE : tex_man.upload_texture(mesh_material.normal_map);
    material.metallic_map = mesh_material.metallic_map.empty() ?
        NO_RESOURCE : tex_man.upload_texture(mesh_material.metallic_map);
    material.roughness_map = mesh_material.roughness_map.empty() ?
        NO_RESOURCE : tex_man.upload_texture(mesh_material.roughness_map);
    material.ambient_occlusion_map = mesh_material.ambient_occlusion_map.empty() ?
        NO_RESOURCE : tex_man.upload_texture(mesh_material.ambient_occlusion_map);

    ResourceID id = m_context.renderer().upload_material(material);
    m_material_ids.insert(id);
    return id;
}

void MaterialManager::free_materials(
    size_t count,
    ResourceID const * ids
) {
    ResourceID const * curr = ids;
    ResourceID const * end = ids + count;
    while (curr != end) {
        free_material(*curr);
        ++curr;
    }
}

void MaterialManager::free_material(ResourceID id) {
    Material const & mat = get_material(id);
    if (mat.albedo_map != NO_RESOURCE) {
        m_context.texture_manager().free_texture_ref(mat.albedo_map);
    }
    if (mat.normal_map != NO_RESOURCE) {
        m_context.texture_manager().free_texture_ref(mat.normal_map);
    }
    if (mat.metallic_map != NO_RESOURCE) {
        m_context.texture_manager().free_texture_ref(mat.metallic_map);
    }
    if (mat.roughness_map != NO_RESOURCE) {
        m_context.texture_manager().free_texture_ref(mat.roughness_map);
    }
    if (mat.ambient_occlusion_map != NO_RESOURCE) {
        m_context.texture_manager().free_texture_ref(mat.ambient_occlusion_map);
    }

    m_context.renderer().free_material(id);
    m_material_ids.erase(id);
}

Material const & MaterialManager::get_material(ResourceID id) const
{ return m_context.renderer().get_material(id); }

Material & MaterialManager::get_material(ResourceID id)
{ return m_context.renderer().get_material(id); }
