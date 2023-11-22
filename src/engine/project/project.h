#ifndef PRT3_PROJECT_H
#define PRT3_PROJECT_H

#include "src/util/uuid.h"
#include "src/engine/component/script/script.h"

#include <unordered_set>
#include <vector>
#include <string>
#include <fstream>

namespace prt3 {

class Scene;

class Project {
public:
    Project() {}
    ~Project();

    void serialize(std::ostream & out) const;
    void deserialize(std::istream & in);

    void on_game_start(Scene & initial_scene);
    void on_game_end(Scene & scene);

    void add_autoload_script(UUID uuid) { m_autoload_scripts.insert(uuid); }
    void remove_autoload_script(UUID uuid) { m_autoload_scripts.erase(uuid); }

    std::vector<Script*> const & active_scripts() const
    { return m_active_scripts; }

    std::unordered_set<UUID> const & autoload_scripts() const
    { return m_autoload_scripts; }

    void set_main_scene_path(std::string const & path)
    { m_main_scene_path = path; }

    std::string const & main_scene_path() const { return m_main_scene_path; }

private:
    std::unordered_set<UUID> m_autoload_scripts;
    std::vector<Script*> m_active_scripts;
    std::string m_main_scene_path;
};

}

#endif // PRT3_PROJECT_H
