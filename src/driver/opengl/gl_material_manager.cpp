#include "gl_material_manager.h"

#include "src/driver/opengl/gl_shader_utility.h"

using namespace prt3;

GLMaterialManager::GLMaterialManager(GLTextureManager & texture_manager)
 : m_texture_manager{texture_manager} {
}

GLMaterialManager::~GLMaterialManager() {
    delete m_standard_shader;
}

void GLMaterialManager::init() {
    m_standard_shader = new GLShader(
        "assets/shaders/opengl/standard.vs",
        "assets/shaders/opengl/standard.fs"
    );

    m_standard_animated_shader = new GLShader(
        "assets/shaders/opengl/standard_animated.vs",
        "assets/shaders/opengl/standard.fs"
    );

    m_transparent_shader = new GLShader(
        "assets/shaders/opengl/standard.vs",
        "assets/shaders/opengl/transparent.fs"
    );

    m_transparent_animated_shader = new GLShader(
        "assets/shaders/opengl/standard_animated.vs",
        "assets/shaders/opengl/transparent.fs"
    );

    m_wireframe_shader = new GLShader(
        "assets/shaders/opengl/wireframe.vs",
        "assets/shaders/opengl/wireframe.fs"
    );
    upload_default_material();
}

ResourceID GLMaterialManager::upload_material(
    Material const & material
) {
    GLTextureManager & tm = m_texture_manager;

    GLuint albedo_map = material.albedo_map == NO_RESOURCE ?
        tm.texture_1x1_0xffffffff() : tm.get_texture(material.albedo_map);
    GLuint normal_map = material.normal_map == NO_RESOURCE ?
        tm.texture_1x1_0x0000ff() : tm.get_texture(material.normal_map);
    GLuint roughness_map = material.roughness_map == NO_RESOURCE ?
        tm.texture_1x1_0xff() : tm.get_texture(material.roughness_map);
    GLuint metallic_map = material.metallic_map == NO_RESOURCE ?
        tm.texture_1x1_0xff() : tm.get_texture(material.metallic_map);

    ResourceID id;
    if (m_free_ids.empty()) {
        id = m_materials.size();
    } else {
        id = m_free_ids.back();
        m_free_ids.pop_back();
    }

    m_materials.emplace(
        std::make_pair(
            id,
            GLMaterial{
                *m_standard_shader,
                *m_standard_animated_shader,
                *m_transparent_shader,
                *m_transparent_animated_shader,
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
    m_materials.erase(id);
    m_free_ids.push_back(id);
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
                *m_standard_animated_shader,
                *m_transparent_shader,
                *m_transparent_animated_shader,
                albedo_map,
                normal_map,
                roughness_map,
                metallic_map,
                Material{}
            }
        )
    );
}
