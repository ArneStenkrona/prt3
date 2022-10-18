#include "renderer.h"

#include "src/driver/opengl/gl_renderer.h"

#include "src/engine/core/context.h"

using namespace prt3;

Renderer::Renderer(Context & context,
                   unsigned int width,
                   unsigned int height,
                   float downscale_factor)
 : m_context{context},
   m_window_width{static_cast<int>(width)},
   m_window_height{static_cast<int>(height)},
   m_downscale_factor{downscale_factor} {
    /* Set SDL attributes */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_window = SDL_CreateWindow(
                "prt3",
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                width,
                height,
                SDL_WINDOW_OPENGL        |
                    SDL_WINDOW_RESIZABLE |
                    SDL_WINDOW_SHOWN/*     |
                    SDL_WINDOW_ALLOW_HIGHDPI*/);

    m_render_backend = new GLRenderer(m_window, downscale_factor);
    m_input.init(m_window, m_render_backend);
}

Renderer::~Renderer() {
    delete m_render_backend;
}

void Renderer::render(RenderData const & render_data,
                      bool render_gui) {
    m_render_backend->render(render_data, render_gui);
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