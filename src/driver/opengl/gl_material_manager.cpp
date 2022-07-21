#include "gl_material_manager.h"

#include "src/driver/opengl/gl_shader_utility.h"

using namespace prt3;

GLMaterialManager::GLMaterialManager(GLTextureManager & texture_manager)
 : m_texture_manager{texture_manager}
{
    m_standard_shader = glshaderutility::create_shader(
        "assets/shaders/opengl/standard.vs",
        "assets/shaders/opengl/standard.fs"
    );
}

ResourceID GLMaterialManager::upload_material(Model::Material const & material) {
    GLTextureManager & tm = m_texture_manager;
    GLuint albedo_map = tm.retrieve_texture(material.albedo_map.c_str(),
                                            tm.texture_1x1_0xffffffff());
    GLuint normal_map = tm.retrieve_texture(material.normal_map.c_str(),
                                            tm.texture_1x1_0x0000ff());
    GLuint roughness_map = tm.retrieve_texture(material.roughness_map.c_str(),
                                               tm.texture_1x1_0x80());
    GLuint metallic_map = tm.retrieve_texture(material.metallic_map.c_str(),
                                              tm.texture_1x1_0x80());

    ResourceID id = m_materials.size();
    m_materials.emplace_back(
        m_standard_shader,
        albedo_map,
        normal_map,
        roughness_map,
        metallic_map,
        material.albedo,
        material.metallic,
        material.roughness
    );

    return id;
}
