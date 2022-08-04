#ifndef PRT3_RENDER_BACKEND_H
#define PRT3_RENDER_BACKEND_H

#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/resources.h"
#include "src/engine/rendering/postprocessing_pass.h"

#include <vector>

namespace prt3 {

class RenderBackend {
public:
    virtual ~RenderBackend() {};

    virtual void render(RenderData const & render_data) = 0;
    virtual void upload_model(ModelManager::ModelHandle model_handle,
                              Model const & model,
                              ModelResource & resource) = 0;
    // virtual void set_postprocessing_shader(const char * fragment_shader_path) = 0;
    virtual void set_postprocessing_chain(
        std::vector<PostProcessingPass> const & chain_info) = 0;
private:
};

} // namespace prt3

#endif
