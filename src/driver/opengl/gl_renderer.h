#ifndef PRT3_GL_RENDERER_H
#define PRT3_GL_RENDERER_H

#include "src/driver/render_backend.h"
#include "src/driver/opengl/gl_shader.h"
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

    virtual void render(
        RenderData const & render_data,
        bool editor
    );

    virtual void upload_model(
        ModelHandle handle,
        Model const & model,
        ModelResource & resource)
    { m_model_manager.upload_model(handle, model, resource); }

    virtual void free_model(
        ModelHandle handle,
        ModelResource const & resource
    )
    { m_model_manager.free_model(handle, resource); }


    virtual void set_postprocessing_chains(
        PostProcessingChain const & scene_chain,
        PostProcessingChain const & editor_chain
    );

    virtual void process_input_events(std::vector<SDL_Event> const & events) {
        for (SDL_Event const & event : events) {
            ImGui_ImplSDL2_ProcessEvent(&event);
        }
    }

    virtual NodeID get_selected(int x, int y);

    virtual Material const & get_material(ResourceID id) const
    { return m_material_manager.get_material(id); }

    virtual Material & get_material(ResourceID id)
    { return m_material_manager.get_material(id); }

private:
    SDL_Window * m_window;
    float m_downscale_factor;

    GLTextureManager m_texture_manager;
    GLMaterialManager m_material_manager;
    GLModelManager m_model_manager;

    GLPostProcessingChain m_scene_postprocessing_chain;
    GLPostProcessingChain m_editor_postprocessing_chain;

    GLSourceBuffers m_source_buffers;

    GLShader * m_selection_shader;

    uint32_t m_frame = 0; // will overflow after a few years

    void render_framebuffer(
        RenderData const & render_data,
        GLuint framebuffer,
        bool selection_pass
    );

    void render_gui();

    void bind_light_data(
        GLShader const & shader,
        LightRenderData const & light_data
    );

    void bind_mesh_and_camera_data(
        GLShader const & shader,
        MeshRenderData const & mesh_data,
        CameraRenderData const & camera_data
    );

    void bind_material_data(
        GLShader const & shader,
        GLMaterial const & material
    );
};

} // namespace prt3

#endif
