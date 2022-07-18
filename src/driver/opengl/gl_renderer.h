#ifndef PRT3_GL_RENDERER_H
#define PRT3_GL_RENDERER_H

#include "src/driver/render_backend.h"
#include "src/driver/opengl/gl_mesh.h"
#include "src/driver/opengl/gl_shader.h"
#include "src/driver/opengl/gl_material.h"
#include "src/driver/opengl/gl_texture_manager.h"
#include "src/driver/opengl/gl_material_manager.h"
#include "src/driver/opengl/gl_model_manager.h"
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
private:
    SDL_Window * m_window;

    GLTextureManager m_texture_manager;
    GLMaterialManager m_material_manager;
    GLModelManager m_model_manager;

};

} // namespace prt3

#endif
