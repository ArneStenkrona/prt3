#ifndef DDS_INTERACTABLE_H
#define DDS_INTERACTABLE_H

#include "src/daedalus/gui/strings.h"
#include "src/engine/component/script/script.h"
#include "src/engine/scene/scene.h"

#include <vector>

namespace dds {

class GameState;

struct InteractData {
    bool on;
};

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

    void interact(prt3::Scene & scene);

    int32_t priority() const { return m_priority; }
    int32_t & priority() { return m_priority; }

    StringIDType get_string_id() const
    { return m_data.on ? m_string_id_on : m_string_id_off; }

    StringIDType string_id_off() const { return m_string_id_off; }
    StringIDType & string_id_off() { return m_string_id_off; }

    StringIDType string_id_on() const { return m_string_id_on; }
    StringIDType & string_id_on() { return m_string_id_on; }

private:
    std::vector<prt3::NodeID> m_triggers;
    GameState * m_game_state;
    prt3::SignalString m_signal;
    int32_t m_priority = 0;
    StringIDType m_string_id_off = 0;
    StringIDType m_string_id_on = 1;  // TODO: remove this hack, assign properly
    InteractData m_data;

    static prt3::SignalString s_signal;

    void init_signal();

REGISTER_SCRIPT_BEGIN(Interactable, interactable, 14936290493767263686)
REGISTER_SERIALIZED_FIELD(m_priority)
REGISTER_SERIALIZED_FIELD(m_string_id_off)
REGISTER_SERIALIZED_FIELD(m_string_id_on)
REGISTER_SCRIPT_END()
};

} // namespace dds

#endif // DDS_INTERACTABLE_H
