#ifndef PRT3_GL_MODEL_MANAGER_H
#define PRT3_GL_MODEL_MANAGER_H

#include "src/driver/opengl/gl_mesh.h"
#include "src/driver/opengl/gl_material_manager.h"
#include "src/engine/rendering/model_manager.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <unordered_map>

namespace prt3 {

class GLModelManager {
public:
    GLModelManager(GLMaterialManager & material_manager);

    void upload_model(ModelManager::ModelHandle model_handle,
                      Model const & model,
                      ModelResource & resource);

    std::vector<GLMesh> const & meshes() const { return m_meshes; }

private:
    struct ModelBufferHandles {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
    };
    GLMaterialManager & m_material_manager;

    std::unordered_map<ModelManager::ModelHandle, ModelBufferHandles> m_buffer_handles;

    std::vector<GLMesh> m_meshes;
};

} // namespace prt3

#endif
