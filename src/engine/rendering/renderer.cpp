#include "renderer.h"

#include "src/driver/opengl/gl_renderer.h"

#include "src/engine/core/context.h"

#include "imgui_internal.h"

using namespace prt3;

Renderer::Renderer(
    Context & context,
    unsigned int width,
    unsigned int height,
    float downscale_factor
)
 : m_context{context},
   m_window_width{static_cast<int>(width)},
   m_window_height{static_cast<int>(height)},
   m_downscale_factor{downscale_factor} {

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

    /* Render backend */
    m_render_backend = new GLRenderer(
        m_window,
        downscale_factor
    );
}

Renderer::~Renderer() {
    delete m_render_backend;
    ImGui::DestroyContext();
}

void Renderer::upload_model(
    ModelHandle handle,
    Model const & model,
    ModelResource & resource
) {
    m_render_backend->upload_model(handle, model, resource);
}

void Renderer::set_window_size(int w, int h) {
    glfwSetWindowSize(m_window, w, h);
    m_context.edit_scene().update_window_size(w, h);
    m_window_width = w;
    m_window_height = h;
}