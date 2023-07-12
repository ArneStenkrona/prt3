#ifndef PRT3_GL_RENDERER_H
#define PRT3_GL_RENDERER_H

#include "src/driver/render_backend.h"
#include "src/driver/opengl/gl_shader.h"
#include "src/driver/opengl/gl_mesh.h"
#include "src/driver/opengl/gl_material.h"
#include "src/driver/opengl/gl_texture_manager.h"
#include "src/driver/opengl/gl_material_manager.h"
#include "src/driver/opengl/gl_model_manager.h"
#include "src/driver/opengl/gl_postprocessing_chain.h"
#include "src/driver/opengl/gl_source_buffers.h"
#include "src/engine/rendering/model_manager.h"

#include "backends/imgui_impl_glfw.h"

#define GL_GLEXT_PROTOTYPES 1
#include <GLES3/gl3.h>

#include <array>

namespace prt3 {

class GLRenderer : public RenderBackend {
public:
    GLRenderer(
        GLFWwindow * window,
        float downscale_factor
    );

    virtual ~GLRenderer();

    virtual void prepare_imgui_rendering();

    virtual void render(
        RenderData const & render_data,
        bool editor
    );

    virtual void upload_model(
        ModelHandle handle,
        Model const & model,
        ModelResource & resource)
    { m_model_manager.upload_model(handle, model, resource); }

    virtual void free_model(
        ModelHandle handle,
        ModelResource const & resource
    )
    { m_model_manager.free_model(handle, resource); }

    virtual ResourceID upload_line_mesh(
        glm::vec3 const * vertices,
        size_t n
    )
    { return m_model_manager.upload_line_mesh(vertices, n); }

    virtual void update_line_mesh(
        ResourceID id,
        glm::vec3 const * vertices,
        size_t n
    )
    { m_model_manager.update_line_mesh(id, vertices, n); }

    virtual void free_line_mesh(ResourceID id)
    { m_model_manager.free_line_mesh(id); }

    virtual void set_postprocessing_chains(
        PostProcessingChain const & scene_chain,
        PostProcessingChain const & editor_chain
    );

    virtual NodeID get_selected(int x, int y);

    virtual ResourceID upload_material(Material const & material)
    { return m_material_manager.upload_material(material); }

    virtual Material const & get_material(ResourceID id) const
    { return m_material_manager.get_material(id); }

    virtual Material & get_material(ResourceID id)
    { return m_material_manager.get_material(id); }

private:
    GLFWwindow * m_window;
    float m_downscale_factor;

    GLTextureManager m_texture_manager;
    GLMaterialManager m_material_manager;
    GLModelManager m_model_manager;

    GLPostProcessingChain m_scene_postprocessing_chain;
    GLPostProcessingChain m_editor_postprocessing_chain;

    GLSourceBuffers m_source_buffers;

    GLShader * m_selection_shader;
    GLShader * m_animated_selection_shader;
    GLShader * m_transparency_blend_shader;

    uint32_t m_frame = 0; // will overflow after a few years

    enum PassType : unsigned {
        opaque,
        transparent,
        selection
    };

    void render_framebuffer(
        RenderData const & render_data,
        bool editor,
        PassType type
    );

    void render_imgui();

    void bind_light_data(
        GLShader const & shader,
        LightRenderData const & light_data
    );

    void bind_transform_and_camera_data(
        GLShader const & shader,
        glm::mat4 const & transform,
        CameraRenderData const & camera_data
    );

    void bind_node_data(
        GLShader const & shader,
        NodeData node_id
    );

    void bind_material_data(
        GLShader const & shader,
        GLMaterial const & material
    );

    void bind_bone_data(
        GLShader const & shader,
        BoneData const & bone_data
    );
};

} // namespace prt3

#endif
