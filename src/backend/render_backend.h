#ifndef PRT3_RENDER_BACKEND_H
#define PRT3_RENDER_BACKEND_H

#include "src/engine/rendering/material.h"
#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/texture.h"
#include "src/engine/rendering/postprocessing_chain.h"
#include "src/engine/rendering/postprocessing_pass.h"
#include "src/engine/scene/node.h"

#include <GLFW/glfw3.h>

#include <vector>

namespace prt3 {

class RenderBackend {
public:
    virtual ~RenderBackend() {};

    virtual void prepare_imgui_rendering() = 0;

    virtual void render(
        RenderData & render_data,
        bool editor
    ) = 0;

    virtual void upload_model(
        ModelHandle handle,
        Model const & model,
        std::vector<ResourceID> & mesh_resource_ids
    ) = 0;

    virtual void free_model(
        ModelHandle handle,
        std::vector<ResourceID> const & mesh_resource_ids
    ) = 0;

    virtual ResourceID upload_pos_mesh(
        glm::vec3 const * vertices,
        size_t n
    ) = 0;

    virtual void update_pos_mesh(
        ResourceID id,
        glm::vec3 const * vertices,
        size_t n
    ) = 0;

    virtual void free_pos_mesh(ResourceID id) = 0;

    virtual void set_postprocessing_chains(
        PostProcessingChain const & scene_chain,
        PostProcessingChain const & editor_chain
    ) = 0;

    virtual NodeID get_selected(int x, int y) = 0;

    virtual ResourceID upload_material(Material const & material) = 0;
    virtual void free_material(ResourceID id) = 0;

    virtual Material const & get_material(ResourceID id) const = 0;
    virtual Material & get_material(ResourceID id) = 0;

    virtual ResourceID upload_texture(TextureData const & data) = 0;
    virtual void free_texture(ResourceID id) = 0;
private:
};

} // namespace prt3

#endif
