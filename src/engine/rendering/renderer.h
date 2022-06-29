#ifndef PRT3_RENDERER_H
#define PRT3_RENDERER_H

#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/model.h"

#include <vector>

namespace prt3 {

class Context;

class Renderer {
public:
    Renderer(Context & context);

    void render(std::vector<RenderData> & render_data);

    void upload_model(Model const & model, std::vector<ResourceID> & resource_ids);

private:
    // Context & m_context;
};

}

#endif
