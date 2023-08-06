#include "decal.h"

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
    size_t n_path;
    read_stream(in, n_path);

    if (n_path > 0) {
        static std::string path;
        path.resize(n_path);

        in.read(path.data(), path.size());

        m_texture_id = scene.upload_texture(path);
    }
}

void Decal::serialize(
    std::ostream & out,
    Scene const & scene
) const {
    if (m_texture_id != NO_RESOURCE) {
        TextureManager const & man = scene.texture_manager();
        std::string const & path = man.get_texture_path(m_texture_id);

        write_stream(out, path.size());
        out.write(path.data(), path.size());
    } else {
        write_stream(out, 0);
    }
}