#include "project.h"

#include "src/util/serialization_util.h"

using namespace prt3;

void Project::serialize(std::ostream & out) const {
    write_string(out, m_main_scene_path);

    write_stream(out, m_autoload_scripts.size());
    for (UUID uuid : m_autoload_scripts) {
        write_stream(out, uuid);
    }
}

void Project::deserialize(std::istream & in) {
    read_string(in, m_main_scene_path);

    size_t n_scripts;
    read_stream(in, n_scripts);
    for (size_t i = 0; i < n_scripts; ++i) {
        UUID uuid;
        read_stream(in, uuid);
        m_autoload_scripts.insert(uuid);
    }
}