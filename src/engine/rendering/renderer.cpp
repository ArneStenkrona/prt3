#include "renderer.h"

#include "src/driver/opengl/gl_renderer.h"

#include "src/engine/core/context.h"

using namespace prt3;

Renderer::Renderer(Context & context)
 : m_context{context},
   m_window_width{640},
   m_window_height{480} {
    SDL_CreateWindowAndRenderer(m_window_width, m_window_height, 0, &m_window, nullptr);
    m_render_backend = new GLRenderer(m_window);
    m_input.init(m_window);
}

Renderer::~Renderer() {
    delete m_render_backend;
}

void Renderer::render(RenderData const & render_data) {
    m_render_backend->render(render_data);
}

void Renderer::upload_model(ModelManager::ModelHandle handle,
                            Model   const & model,
                            ModelResource & resource) {
    m_render_backend->upload_model(handle, model, resource);
}

void Renderer::set_window_size(int w, int h) {
    SDL_SetWindowSize(m_window, w, h);
    m_context.current_scene().update_window_size(w, h);
    m_window_width = w;
    m_window_height = h;
}