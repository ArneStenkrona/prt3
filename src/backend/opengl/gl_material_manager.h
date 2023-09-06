#ifndef PRT3_GL_MATERIAL_MANAGER_H
#define PRT3_GL_MATERIAL_MANAGER_H

#include "src/backend/opengl/gl_texture_manager.h"
#include "src/backend/opengl/gl_material.h"
#include "src/backend/opengl/gl_shader.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/model.h"
#include "src/engine/rendering/material.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <unordered_map>

namespace prt3 {

class GLMaterialManager {
public:
    GLMaterialManager(GLTextureManager & texture_manager);
    ~GLMaterialManager();
    void init();

    ResourceID upload_material(Material const & material);
    void free_material(ResourceID id);

    std::unordered_map<ResourceID, GLMaterial> const & materials() const
    { return m_materials; }

    Material const & get_material(ResourceID id) const
    { return m_materials.at(id).material(); }

    Material & get_material(ResourceID id)
    { return m_materials.at(id).material(); }

    GLShader const & standard_shader() const { return *m_standard_shader; }
    GLShader const & standard_animated_shader() const
    { return *m_standard_animated_shader; }
    GLShader const & wireframe_shader() const { return *m_wireframe_shader; }
private:
    GLTextureManager & m_texture_manager;

    std::unordered_map<ResourceID, GLMaterial> m_materials;
    std::vector<ResourceID> m_free_ids;

    GLShader * m_standard_shader = nullptr;
    GLShader * m_standard_animated_shader = nullptr;
    GLShader * m_transparent_shader = nullptr;
    GLShader * m_transparent_animated_shader = nullptr;
    GLShader * m_wireframe_shader = nullptr;

    void upload_default_material();
};

} // namespace prt3;

#endif