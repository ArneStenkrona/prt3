#ifndef PRT3_GL_RENDERER_H
#define PRT3_GL_RENDERER_H

#include "src/driver/render_backend.h"
#include "src/driver/opengl/gl_mesh.h"
#include "src/driver/opengl/gl_material.h"
#include "src/driver/opengl/gl_texture_manager.h"
#include "src/driver/opengl/gl_material_manager.h"
#include "src/driver/opengl/gl_model_manager.h"
#include "src/driver/opengl/gl_postprocessing_chain.h"
#include "src/driver/opengl/gl_source_buffers.h"
#include "src/engine/rendering/model_manager.h"

#include "backends/imgui_impl_sdl.h"

#include <SDL.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

namespace prt3 {

class GLRenderer : public RenderBackend {
public:
    GLRenderer(SDL_Window * window,
               float downscale_factor);
    virtual ~GLRenderer();

    virtual void prepare_gui_rendering();

    virtual void render(RenderData const & render_data,
                        bool gui);
    virtual void upload_model(
        ModelManager::ModelHandle model_handle,
        Model const & model,
        ModelResource & resource) {
        m_model_manager.upload_model(model_handle, model, resource);
    }

    virtual void set_postprocessing_chain(
        PostProcessingChain const & chain);

    virtual void process_input_event(void const * event) {
        ImGui_ImplSDL2_ProcessEvent(static_cast<SDL_Event const*>(event));
    }

private:
    SDL_Window * m_window;
    float m_downscale_factor;

    GLTextureManager m_texture_manager;
    GLMaterialManager m_material_manager;
    GLModelManager m_model_manager;

    GLPostProcessingChain m_postprocessing_chain;

    GLSourceBuffers m_source_buffers;

    void render_framebuffer(
        RenderData const & render_data,
        GLuint framebuffer
    );

    void render_gui();
};

} // namespace prt3

#endif
