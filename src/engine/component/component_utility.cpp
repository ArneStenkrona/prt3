#include "component_utility.h"

#include "src/util/serialization_util.h"

using namespace prt3;

void prt3::serialize_texture(
    std::ostream & out,
    Scene const & scene,
    ResourceID tex_id
) {
    if (tex_id != NO_RESOURCE) {
        TextureManager const & man = scene.texture_manager();
        std::string const & path = man.get_texture_path(tex_id);

        write_stream(out, path.size());
        out.write(path.data(), path.size());
    } else {
        write_stream(out, 0);
    }
}

ResourceID prt3::deserialize_texture(std::istream & in, Scene & scene) {
    size_t n_path;
    read_stream(in, n_path);

    if (n_path > 0) {
        static std::string path;
        path.resize(n_path);

        in.read(path.data(), path.size());

        return scene.upload_texture(path);
    }

    return NO_RESOURCE;
}
