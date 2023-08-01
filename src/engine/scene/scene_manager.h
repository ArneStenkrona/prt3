#ifndef PRT3_SCENE_MANAGER_H
#define PRT3_SCENE_MANAGER_H

#include "src/engine/scene/scene.h"

namespace prt3 {

typedef int32_t TransitionState;
constexpr TransitionState NO_TRANSITION = -1;

class Context;
class SceneManager {
public:
    SceneManager(Context & context);

    void queue_scene(std::string const & path);
    void reset_queue() { m_queued_scene_path = ""; }

    bool scene_queued() const { return m_queued_scene_path != ""; }

    bool & fade_transition() { return m_fade_transition; }

    std::string const & queued_scene_path() const
    { return m_queued_scene_path; }

    void exclude_node_from_fade(NodeID id)
    { m_fade_exclude_set.insert(id); }

private:
    Context & m_context;
    std::string m_queued_scene_path;
    bool m_fade_transition = true;

    std::unordered_set<NodeID> m_fade_exclude_set;

    TransitionState load_scene_if_queued(
        TransitionState state,
        Scene & scene,
        Scene & edit_scene
    );

    void transition_scene();
    TransitionState transition_fade(TransitionState state, Scene & scene);

    void set_tint(
        Scene & scene,
        bool active,
        glm::vec3 tint,
        std::unordered_set<NodeID> const & exclude
    );

    friend class Context;
};

} // namespace prt3

#endif // PRT3_SCENE_LOADER
