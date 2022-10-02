#ifndef PRT3_GL_RENDERER_H
#define PRT3_GL_RENDERER_H

#include "src/driver/render_backend.h"
#include "src/driver/opengl/gl_mesh.h"
#include "src/driver/opengl/gl_material.h"
#include "src/driver/opengl/gl_texture_manager.h"
#include "src/driver/opengl/gl_material_manager.h"
#include "src/driver/opengl/gl_model_manager.h"
#include "src/driver/opengl/gl_postprocessing_chain.h"
// #include "src/driver/opengl/gl_postprocessing_pass.h"
#include "src/engine/rendering/model_manager.h"

#include <SDL.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

namespace prt3 {

class GLRenderer : public RenderBackend {
public:
    GLRenderer(SDL_Window * window,
               float downscale_factor);
    virtual ~GLRenderer();

    virtual void render(RenderData const & render_data);
    virtual void upload_model(ModelManager::ModelHandle model_handle,
                              Model const & model,
                              ModelResource & resource)
        { m_model_manager.upload_model(model_handle, model, resource); }

    virtual void set_postprocessing_chain(
        std::vector<PostProcessingPass> const & chain_info);
private:
    SDL_Window * m_window;
    float m_downscale_factor;

    GLTextureManager m_texture_manager;
    GLMaterialManager m_material_manager;
    GLModelManager m_model_manager;

    GLuint m_framebuffer;
    GLuint m_color_texture;
    GLuint m_depth_texture;

    GLuint m_normal_framebuffer;
    GLuint m_normal_texture;
    GLuint m_normal_depth_texture;

    GLPostProcessingChain m_postprocessing_chain;

    // re-use same object to avoid too many heap allocations
    mutable std::unordered_map<ResourceID, std::vector<MeshRenderData>>
        m_material_queues;

    void generate_framebuffer(GLuint & framebuffer,
                              GLuint & render_texture,
                              GLuint & depth_texture);

    void render_framebuffer(RenderData const & render_data,
                            GLuint framebuffer,
                            bool normal_pass);
};

} // namespace prt3

#endif
