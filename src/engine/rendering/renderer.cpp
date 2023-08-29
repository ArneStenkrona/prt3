#include "renderer.h"

#include "src/backend/dummy/dummy_renderer.h"
#include "src/backend/opengl/gl_renderer.h"
#include "src/engine/core/backend_type.h"
#include "src/engine/core/context.h"

using namespace prt3;

Renderer::Renderer(
    Context & context,
    unsigned int width,
    unsigned int height,
    float downscale_factor,
    BackendType backend_type
)
 : m_context{context},
   m_window_width{static_cast<int>(width)},
   m_window_height{static_cast<int>(height)},
   m_downscale_factor{downscale_factor} {

    m_is_dummy = backend_type == BackendType::dummy;

    if (!m_is_dummy) {
        /* glfw */
        if (!glfwInit()) {
            PRT3ERROR("Failed to initialize glfw.\n");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        m_window = glfwCreateWindow(
            m_window_width,
            m_window_height,
            "prt3",
            NULL,
            NULL
        );

        if (!m_window) {
            glfwTerminate();
            PRT3ERROR("Failed to create glfw window.\n");
        }

        glfwMakeContextCurrent(m_window);

        /* Input */
        m_input.init(m_window);

        /* ImGui */
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    /* Render backend */
    switch (backend_type) {
        case (BackendType::dummy): {
            m_render_backend = new DummyRenderer();
            break;
        }
        case (BackendType::wasm): {
            m_render_backend = new GLRenderer(
                m_window,
                downscale_factor
            );
            break;
        }
    }
}

Renderer::~Renderer() {
    delete m_render_backend;
    if (!m_is_dummy) {
        ImGui::DestroyContext();
    }
}

void Renderer::set_window_size(int w, int h) {
    glfwSetWindowSize(m_window, w, h);
    m_context.edit_scene().update_window_size(w, h);
    m_window_width = w;
    m_window_height = h;
}