#ifndef PRT3_GL_RENDERER_H
#define PRT3_GL_RENDERER_H

#include "src/driver/render_backend.h"
#include "src/driver/opengl/gl_mesh.h"
#include "src/driver/opengl/gl_shader.h"
#include "src/driver/opengl/gl_material.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/model_manager.h"

#include <SDL.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>

#include <vector>
#include <unordered_map>

namespace prt3 {

class GLRenderer : public RenderBackend {
public:
    GLRenderer(SDL_Window * window);
    virtual ~GLRenderer();

    virtual void render(RenderData const & render_data);
    virtual void upload_model(ModelManager::ModelHandle model_handle,
                              Model const & model,
                              ModelResource & resource);
private:
    struct ModelBufferHandles {
        GLuint vao;
        GLuint vbo;
        GLuint ebo;
    };
    SDL_Window * m_window;

    std::unordered_map<ModelManager::ModelHandle, ModelBufferHandles> m_buffer_handles;

    GLShader m_standard_shader;
    std::vector<GLMaterial> m_materials;
    std::vector<GLMesh> m_meshes;

    GLuint m_default_texture;
    std::unordered_map<std::string, GLuint> m_textures;

    ResourceID upload_material(Model::Material const & material);
    GLuint retrieve_texture(char const * path);
    void load_texture(char const * path);
    void load_default_texture();
};

} // namespace prt3

#endif
