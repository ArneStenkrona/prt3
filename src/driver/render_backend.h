#ifndef PRT3_RENDER_BACKEND_H
#define PRT3_RENDER_BACKEND_H

#include "src/engine/rendering/model.h"
#include "src/engine/rendering/model_manager.h"
#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/resources.h"

#include <vector>

namespace prt3 {

class RenderBackend {
public:
    static constexpr ResourceID DEFAULT_MATERIAL_ID = 0;

    virtual ~RenderBackend() {};

    virtual void render(RenderData const & render_data) = 0;
    virtual void upload_model(ModelManager::ModelHandle model_handle,
                              Model const & model,
                              ModelResource & resource) = 0;
private:
};

} // namespace prt3

#endif