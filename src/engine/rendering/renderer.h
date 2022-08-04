#ifndef PRT3_RENDERER_H
#define PRT3_RENDERER_H

#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/postprocessing_pass.h"
#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/rendering/resources.h"
#include "src/driver/render_backend.h"
#include "src/engine/core/input.h"

#include <SDL.h>

#include <vector>

namespace prt3 {

class Context;

class Renderer {
public:
    Renderer(Context & context,
             unsigned int width,
             unsigned int height,
             float downscale_factor);
    Renderer(Renderer const &) = delete;
    ~Renderer();

    void render(RenderData const & render_data);
    void upload_model(ModelManager::ModelHandle handle,
                      Model const & model,
                      ModelResource & resource);
    // void set_postprocesing_shader(const char * fragment_shader_path)
    //     { m_render_backend->set_postprocessing_shader(fragment_shader_path); }

    void set_postprocessing_chain(
        std::vector<PostProcessingPass> const & chain_info)
        { m_render_backend->set_postprocessing_chain(chain_info); }

    Input & input() { return m_input; }

    int window_width() const { return m_window_width; }
    int window_height() const { return m_window_height; }
    float downscale_factor() const { return m_downscale_factor; }

private:
    RenderBackend * m_render_backend;
    SDL_Window * m_window;
    Input m_input;

    Context & m_context;

    int m_window_width;
    int m_window_height;
    float m_downscale_factor;

    void set_window_size(int w, int h);
};

} // namespace prt3

#endif
