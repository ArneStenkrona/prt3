#include "decal.h"

#include "src/engine/component/component_utility.h"
#include "src/engine/scene/scene.h"

using namespace prt3;

Decal::Decal(Scene & /*scene*/, NodeID node_id)
 : m_node_id{node_id}
{}

Decal::Decal(Scene & /*scene*/, NodeID node_id, ResourceID texture_id)
 : m_node_id{node_id}, m_texture_id{texture_id}
{}

Decal::Decal(Scene & scene, NodeID node_id, std::istream & in)
 : m_node_id{node_id} {
    m_texture_id = deserialize_texture(in, scene);
    read_stream(in, m_dimensions);
    read_stream(in, m_color);
}

void Decal::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    serialize_texture(out, scene, m_texture_id);
    write_stream(out, m_dimensions);
    write_stream(out, m_color);
}


void Decal::collect_render_data(
    std::vector<Decal> const & components,
    std::vector<Transform> const & global_transforms,
    std::vector<DecalRenderData> & data
) {
    for (Decal const & decal : components) {
        if (decal.texture_id() == NO_RESOURCE) continue;
        Transform tform = global_transforms[decal.node_id()];
        tform.scale *= decal.dimensions();

        data.push_back({});
        data.back().transform = tform.to_matrix();
        data.back().color = decal.m_color;
        data.back().texture = decal.texture_id();
    }
}
