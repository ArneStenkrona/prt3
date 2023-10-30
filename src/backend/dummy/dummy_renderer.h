#ifndef PRT3_DUMMY_RENDERER_H
#define PRT3_DUMMY_RENDERER_H

#include "src/backend/render_backend.h"

#include <GLFW/glfw3.h>

#include <vector>
#include <unordered_map>

namespace prt3 {

class DummyRenderer : public RenderBackend {
public:
    DummyRenderer() {}

    virtual ~DummyRenderer() {};

    virtual void prepare_imgui_rendering() {};

    virtual void render(
        RenderData &,
        bool
    ) {}

    virtual void upload_model(
        ModelHandle handle,
        Model const & model,
        std::vector<ResourceID> & mesh_resource_ids
    );

    virtual void free_model(
        ModelHandle,
        std::vector<ResourceID> const &
    ) {}

    virtual ResourceID upload_pos_mesh(
        glm::vec3 const * vertices,
        size_t n
    );

    virtual void update_pos_mesh(
        ResourceID,
        glm::vec3 const *,
        size_t
    ) {}

    virtual void free_pos_mesh(ResourceID) {}

    virtual void set_postprocessing_chains(
        PostProcessingChain const &,
        PostProcessingChain const &
    ) {}

    virtual NodeID get_selected(int x, int y);

    virtual ResourceID upload_material(Material const & material);
    virtual void free_material(ResourceID);

    virtual Material const & get_material(ResourceID id) const;
    virtual Material & get_material(ResourceID id);

    virtual ResourceID upload_texture(TextureData const & data);
    virtual void free_texture(ResourceID) {};
private:
    ResourceID m_mesh_counter = 0;
    std::unordered_map<ResourceID, Material> m_materials;
    ResourceID m_texture_counter = 0;
};

} // namespace prt3

#endif // PRT3_DUMMY_RENDERER_H
