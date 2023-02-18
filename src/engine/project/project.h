#ifndef PRT3_PROJECT_H
#define PRT3_PROJECT_H

#include "src/util/uuid.h"

#include <unordered_set>
#include <string>
#include <fstream>

namespace prt3 {

class Project {
public:
    Project() {}

    void serialize(std::ostream & out) const;
    void deserialize(std::istream & in);

    void add_autoload_script(UUID uuid) { m_autoload_scripts.insert(uuid); }
    void remove_autoload_script(UUID uuid) { m_autoload_scripts.erase(uuid); }

    std::unordered_set<UUID> const & autoload_scripts() const
    { return m_autoload_scripts; }

    void set_main_scene_path(std::string const & path)
    { m_main_scene_path = path; }

    std::string const & main_scene_path() const { return m_main_scene_path; }

private:

    std::unordered_set<UUID> m_autoload_scripts;
    std::string m_main_scene_path;
};

}

#endif // PRT3_PROJECT_H
