#ifndef PRT3_GL_MATERIAL_MANAGER_H
#define PRT3_GL_MATERIAL_MANAGER_H

#include "src/driver/opengl/gl_texture_manager.h"
#include "src/driver/opengl/gl_material.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/model.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

namespace prt3 {

class GLMaterialManager {
public:
    GLMaterialManager(GLTextureManager & texture_manager);
    void init();

    ResourceID upload_material(Model::Material const & material);

    std::vector<GLMaterial> const & materials() const
    { return m_materials; }

    GLuint standard_shader() const { return m_standard_shader; }

private:
    GLTextureManager & m_texture_manager;

    std::vector<GLMaterial> m_materials;

    GLuint m_standard_shader;
};

} // namespace prt3;

#endif
