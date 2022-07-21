#ifndef PRT3_GL_RENDERER_H
#define PRT3_GL_RENDERER_H

#include "src/driver/render_backend.h"
#include "src/driver/opengl/gl_mesh.h"
#include "src/driver/opengl/gl_material.h"
#include "src/driver/opengl/gl_texture_manager.h"
#include "src/driver/opengl/gl_material_manager.h"
#include "src/driver/opengl/gl_model_manager.h"
#include "src/driver/opengl/gl_postprocessing_pass.h"
#include "src/engine/rendering/model_manager.h"

#include <SDL.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>

namespace prt3 {

class GLRenderer : public RenderBackend {
public:
    GLRenderer(SDL_Window * window);
    virtual ~GLRenderer();

    virtual void render(RenderData const & render_data);
    virtual void upload_model(ModelManager::ModelHandle model_handle,
                              Model const & model,
                              ModelResource & resource)
        { m_model_manager.upload_model(model_handle, model, resource); }

    virtual void set_postprocessing_shader(const char * fragment_shader_path);
private:
    SDL_Window * m_window;

    GLTextureManager m_texture_manager;
    GLMaterialManager m_material_manager;
    GLModelManager m_model_manager;

    GLuint m_framebuffer;
    GLuint m_render_texture;
    GLuint m_depth_buffer;

    GLPostProcessingPass m_postprocessing_pass;

    // GLuint m_quad_vertexbuffer;
    // GLuint m_screen_quad_vao;
    // GLuint m_screen_quad_vbo;

    // GLShader m_passthrough_shader;
};

} // namespace prt3

#endif
