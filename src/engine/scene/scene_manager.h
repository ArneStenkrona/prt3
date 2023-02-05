#ifndef PRT3_SCENE_MANAGER_H
#define PRT3_SCENE_MANAGER_H

#include "src/engine/scene/scene.h"

namespace prt3 {

class Context;
class SceneManager {
public:
    SceneManager(Context & context);

    void queue_scene(std::string const & path);
    void reset_queue() { m_queued_scene_path = ""; }

    bool scene_queued() const { return m_queued_scene_path != ""; }

    std::string const & queued_scene_path() const
    { return m_queued_scene_path; }
private:
    // Context & m_context;
    std::string m_queued_scene_path;
};

} // namespace prt3

#endif // PRT3_SCENE_LOADER
