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

    void prepare_gui_rendering() { m_render_backend->prepare_gui_rendering(); }

    void render(RenderData const & render_data, bool editor);

    void upload_model(ModelManager::ModelHandle handle,
                      Model const & model,
                      ModelResource & resource);

    void free_model(
        ModelManager::ModelHandle handle,
        ModelResource const & resource
    )
    { m_render_backend->free_model(handle, resource); }

    NodeID get_selected(int x, int y) {
        return m_render_backend->get_selected(x, y);
    }

    void process_input_events(std::vector<SDL_Event> const & events) {
        m_render_backend->process_input_events(events);
    }

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
