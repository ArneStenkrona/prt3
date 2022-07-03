#include "renderer.h"

#include "src/driver/opengl/gl_renderer.h"

#include "src/engine/core/context.h"

using namespace prt3;

Renderer::Renderer(Context & /*context*/)
//  : m_context{context} {
{
    SDL_CreateWindowAndRenderer(640, 480, 0, &m_window, nullptr);
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
