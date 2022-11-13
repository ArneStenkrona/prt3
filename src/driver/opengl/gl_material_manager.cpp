#include "gl_material_manager.h"

#include "src/driver/opengl/gl_shader_utility.h"

using namespace prt3;

GLMaterialManager::GLMaterialManager(GLTextureManager & texture_manager)
 : m_texture_manager{texture_manager},
   m_standard_shader{nullptr} {
}

GLMaterialManager::~GLMaterialManager() {
    delete m_standard_shader;
}


void GLMaterialManager::init() {
    m_standard_shader = new GLShader(
        "assets/shaders/opengl/standard.vs",
        "assets/shaders/opengl/standard.fs"
    );
    upload_default_material();
}

ResourceID GLMaterialManager::upload_material(Material const & material) {
    GLTextureManager & tm = m_texture_manager;
    GLuint albedo_map = tm.retrieve_texture(
        material.albedo_map.c_str(),
        tm.texture_1x1_0xffffffff()
    );

    GLuint normal_map = tm.retrieve_texture(
        material.normal_map.c_str(),
        tm.texture_1x1_0x0000ff()
    );

    GLuint roughness_map = tm.retrieve_texture(
        material.roughness_map.c_str(),
        tm.texture_1x1_0xff()
    );

    GLuint metallic_map = tm.retrieve_texture(
        material.metallic_map.c_str(),
        tm.texture_1x1_0xff()
    );

    ResourceID id = m_next_material_id;
    ++m_next_material_id;

    m_materials.emplace(
        std::make_pair(
            id,
            GLMaterial{
                *m_standard_shader,
                albedo_map,
                normal_map,
                roughness_map,
                metallic_map,
                material
            }
        )
    );


    return id;
}

void GLMaterialManager::free_material(
    ResourceID id
) {
    GLMaterial const & mat = m_materials.at(id);

    m_texture_manager.attempt_free_texture(mat.albedo_map());
    m_texture_manager.attempt_free_texture(mat.normal_map());
    m_texture_manager.attempt_free_texture(mat.roughness_map());
    m_texture_manager.attempt_free_texture(mat.metallic_map());

    m_materials.erase(id);
}

void GLMaterialManager::upload_default_material() {
    GLTextureManager & tm = m_texture_manager;

    GLuint albedo_map = tm.texture_1x1_0xffffffff();
    GLuint normal_map = tm.texture_1x1_0x0000ff();
    GLuint roughness_map = tm.texture_1x1_0xff();
    GLuint metallic_map = tm.texture_1x1_0xff();

    m_materials.emplace(
        std::make_pair(
            NO_RESOURCE,
            GLMaterial{
                *m_standard_shader,
                albedo_map,
                normal_map,
                roughness_map,
                metallic_map,
                Material{}
            }
        )
    );
}
