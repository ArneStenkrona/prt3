#ifndef DDS_INTERACTABLE_H
#define DDS_INTERACTABLE_H

#include "src/daedalus/game_state/game_state.h"
#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"

#include <vector>

namespace dds {

class Interactable : public prt3::Script {
public:
    Interactable(prt3::Scene & scene, prt3::NodeID node_id);

    Interactable(
        std::istream &,
        prt3::Scene & scene,
        prt3::NodeID node_id
    );

    virtual void on_init(prt3::Scene & scene);
    virtual void on_update(prt3::Scene & scene, float delta_time);

    static prt3::SignalString const & interact_signal() { return s_signal; }

private:
    std::vector<prt3::NodeID> m_triggers;
    GameState * m_game_state;
    prt3::SignalString m_signal;

    static prt3::SignalString s_signal;

REGISTER_SCRIPT(Interactable, interactable, 14936290493767263686)
};

} // namespace dds

#endif // DDS_INTERACTABLE_H
