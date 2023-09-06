#ifndef PRT3_GL_RENDERER_H
#define PRT3_GL_RENDERER_H

#include "src/backend/render_backend.h"
#include "src/backend/opengl/gl_shader.h"
#include "src/backend/opengl/gl_mesh.h"
#include "src/backend/opengl/gl_material.h"
#include "src/backend/opengl/gl_texture_manager.h"
#include "src/backend/opengl/gl_material_manager.h"
#include "src/backend/opengl/gl_model_manager.h"
#include "src/backend/opengl/gl_postprocessing_chain.h"
#include "src/backend/opengl/gl_source_buffers.h"
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

    void prepare_imgui_rendering() final;

    void render(
        RenderData const & render_data,
        bool editor
    ) final;

    void upload_model(
        ModelHandle handle,
        Model const & model,
        std::vector<ResourceID> & mesh_resource_ids
    ) final
    { m_model_manager.upload_model(handle, model, mesh_resource_ids); }

    void free_model(
        ModelHandle handle,
        std::vector<ResourceID> const & mesh_resource_ids
    ) final
    { m_model_manager.free_model(handle, mesh_resource_ids); }

    ResourceID upload_pos_mesh(
        glm::vec3 const * vertices,
        size_t n
    ) final
    { return m_model_manager.upload_pos_mesh(vertices, n); }

    void update_pos_mesh(
        ResourceID id,
        glm::vec3 const * vertices,
        size_t n
    ) final
    { m_model_manager.update_pos_mesh(id, vertices, n); }

    void free_pos_mesh(ResourceID id) final
    { m_model_manager.free_pos_mesh(id); }

    void set_postprocessing_chains(
        PostProcessingChain const & scene_chain,
        PostProcessingChain const & editor_chain
    ) final;

    NodeID get_selected(int x, int y) final;

    ResourceID upload_material(Material const & material) final
    { return m_material_manager.upload_material(material); }
    void free_material(ResourceID id) final
    { m_material_manager.free_material(id); }

    Material const & get_material(ResourceID id) const final
    { return m_material_manager.get_material(id); }

    Material & get_material(ResourceID id) final
    { return m_material_manager.get_material(id); }

    ResourceID upload_texture(TextureData const & data) final
    { return m_texture_manager.upload_texture(data); }
    void free_texture(ResourceID id) final
    { return m_texture_manager.free_texture(id); }

private:
    GLFWwindow * m_window;
    float m_downscale_factor;

    GLTextureManager m_texture_manager;
    GLMaterialManager m_material_manager;
    GLModelManager m_model_manager;

    GLPostProcessingChain m_scene_postprocessing_chain;
    GLPostProcessingChain m_editor_postprocessing_chain;

    GLSourceBuffers m_source_buffers;

    GLShader * m_decal_shader;
    GLShader * m_selection_shader;
    GLShader * m_animated_selection_shader;
    GLShader * m_transparency_blend_shader;

    ResourceID m_decal_mesh;

    uint32_t m_frame = 0; // will overflow after a few years

    enum PassType : unsigned {
        opaque,
        transparent,
        decal,
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

    void bind_decal_data(
        GLShader const & s,
        CameraRenderData const & data
    );

    void bind_node_data(
        GLShader const & shader,
        NodeData node_id
    );

    void bind_material_data(
        GLShader const & shader,
        GLMaterial const & material,
        MaterialOverride const & mat_override
    );

    void bind_bone_data(
        GLShader const & shader,
        BoneData const & bone_data
    );

    void bind_texture(
        GLShader const & s,
        GLVarString const & uniform_str,
        unsigned int location,
        GLuint texture
    );
};

} // namespace prt3

#endif