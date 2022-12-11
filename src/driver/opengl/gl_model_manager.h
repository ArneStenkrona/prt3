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

    void upload_model(ModelHandle handle,
                      Model const & model,
                      ModelResource & resource);

    void free_model(
        ModelHandle handle,
        ModelResource const & resource
    );

    ResourceID upload_line_mesh(std::vector<glm::vec3> const & vertices);

    void update_line_mesh(
        ResourceID id,
        std::vector<glm::vec3> const & vertices
    );

    void free_line_mesh(ResourceID id);

    std::unordered_map<ResourceID, GLMesh> const & meshes() const
    { return m_meshes; }

private:
    struct ModelBufferHandles {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
    };

    struct LineMeshBufferHandles {
        GLuint vao;
        GLuint vbo;
    };
    GLMaterialManager & m_material_manager;

    std::unordered_map<ModelHandle, ModelBufferHandles>
        m_model_buffer_handles;

    std::unordered_map<ResourceID, LineMeshBufferHandles>
        m_line_mesh_buffer_handles;

    std::unordered_map<ResourceID, GLMesh> m_meshes;
    ResourceID m_next_mesh_id = 0;

    void bind_vertex_buffer(Model const & model);
    void bind_boned_vertex_buffer(Model const & model);
};

} // namespace prt3

#endif
