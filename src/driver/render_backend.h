#ifndef PRT3_RENDER_BACKEND_H
#define PRT3_RENDER_BACKEND_H

#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/postprocessing_chain.h"
#include "src/engine/rendering/postprocessing_pass.h"
#include "src/engine/scene/node.h"

#include <SDL.h>

#include <vector>

namespace prt3 {

class RenderBackend {
public:
    virtual ~RenderBackend() {};

    virtual void prepare_imgui_rendering() = 0;

    virtual void render(
        RenderData const & render_data,
        bool editor
    ) = 0;

    virtual void upload_model(
        ModelHandle handle,
        Model const & model,
        ModelResource & resource
    ) = 0;

    virtual void free_model(
        ModelHandle handle,
        ModelResource const & resource
    ) = 0;

    virtual ResourceID upload_line_mesh(std::vector<glm::vec3> const & vertices) = 0;

    virtual void update_line_mesh(
        ResourceID id,
        std::vector<glm::vec3> const & vertices
    ) = 0;

    virtual void free_line_mesh(ResourceID id) = 0;

    virtual void set_postprocessing_chains(
        PostProcessingChain const & scene_chain,
        PostProcessingChain const & editor_chain
    ) = 0;

    virtual void process_input_events(std::vector<SDL_Event> const & events) = 0;

    virtual NodeID get_selected(int x, int y) = 0;

    virtual ResourceID upload_material(Material const & material) = 0;

    virtual Material const & get_material(ResourceID id) const = 0;
    virtual Material & get_material(ResourceID id) = 0;

private:
};

} // namespace prt3

#endif
