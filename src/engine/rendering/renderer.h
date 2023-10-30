#ifndef PRT3_RENDERER_H
#define PRT3_RENDERER_H

#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/postprocessing_pass.h"
#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/material.h"
#include "src/engine/rendering/texture.h"
#include "src/backend/render_backend.h"
#include "src/engine/core/backend_type.h"
#include "src/engine/core/input.h"

#include "backends/imgui_impl_glfw.h"

#include <GLFW/glfw3.h>

#include <vector>

namespace prt3 {

class Context;

class Renderer {
public:
    Renderer(Context & context,
             unsigned int width,
             unsigned int height,
             float downscale_factor,
             BackendType backend_type);
    Renderer(Renderer const &) = delete;
    ~Renderer();

    void prepare_imgui_rendering() { m_render_backend->prepare_imgui_rendering(); }

    void render(
        RenderData & render_data,
        bool editor
    ) {
        m_render_backend->render(render_data, editor);
    }

    void upload_model(
        ModelHandle handle,
        Model const & model,
        std::vector<ResourceID> & mesh_resource_ids
    )
    { m_render_backend->upload_model(handle, model, mesh_resource_ids); }

    void free_model(
        ModelHandle handle,
        std::vector<ResourceID> const & mesh_resource_ids
    )
    { m_render_backend->free_model(handle, mesh_resource_ids); }

    ResourceID upload_pos_mesh(glm::vec3 const * vertices, size_t n)
    { return m_render_backend->upload_pos_mesh(vertices, n); }

    void update_pos_mesh(
        ResourceID id,
        glm::vec3 const * vertices,
        size_t n
    )
    { m_render_backend->update_pos_mesh(id, vertices, n); }

    void free_pos_mesh(
        ResourceID id
    )
    { m_render_backend->free_pos_mesh(id); }

    ResourceID upload_material(Material const & material)
    { return m_render_backend->upload_material(material); }
    void free_material(ResourceID id)
    { m_render_backend->free_material(id); }

    Material & get_material(ResourceID id)
    { return m_render_backend->get_material(id); }

    ResourceID upload_texture(TextureData const & data)
    { return m_render_backend->upload_texture(data); }
    void free_texture(ResourceID id)
    { return m_render_backend->free_texture(id); }

    NodeID get_selected(int x, int y) {
        return m_render_backend->get_selected(x, y);
    }

    Input & input() { return m_input; }

    int window_width() const { return m_window_width; }
    int window_height() const { return m_window_height; }
    float downscale_factor() const { return m_downscale_factor; }

private:
    RenderBackend * m_render_backend;
    GLFWwindow * m_window;
    Input m_input;

    Context & m_context;

    int m_window_width;
    int m_window_height;
    float m_downscale_factor;

    bool m_is_dummy;

    void set_window_size(int w, int h);

    void on_mode_game()
    { ImGui_ImplGlfw_RestoreCallbacks(m_window); }

    void on_mode_editor()
    { ImGui_ImplGlfw_InstallCallbacks(m_window); }

    friend class Engine;
};

} // namespace prt3

#endif
