#ifndef PRT3_RENDERER_H
#define PRT3_RENDERER_H

#include "src/engine/rendering/render_data.h"
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
    Renderer(Context & context);
    Renderer(Renderer const &) = delete;
    ~Renderer();

    void render(RenderData const & render_data);
    void upload_model(ModelManager::ModelHandle handle,
                      Model const & model,
                      ModelResource & resource);

    Input & input() { return m_input; }

private:
    RenderBackend * m_render_backend;
    SDL_Window * m_window;
    Input m_input;

    // Context & m_context;
};

}

#endif
