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
}

void Decal::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    serialize_texture(out, scene, m_texture_id);
}
