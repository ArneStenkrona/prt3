#include "interactable.h"

#include "src/daedalus/input_mapping/input_mapping.h"

#include <inttypes.h>

using namespace dds;

prt3::SignalString Interactable::s_signal = "interact";

Interactable::Interactable(prt3::Scene & scene, prt3::NodeID node_id)
 : prt3::Script(scene, node_id) {
    m_signal = s_signal;
    snprintf(
        m_signal.data() + s_signal.len(),
        m_signal.writeable_size() - s_signal.len(),
        "%" PRId32,
        node_id
    );
}

Interactable::Interactable(
    std::istream &,
    prt3::Scene & scene,
    prt3::NodeID node_id
)
 : prt3::Script(scene, node_id) {
    m_signal = s_signal;
    snprintf(
        m_signal.data() + s_signal.len(),
        m_signal.writeable_size() - s_signal.len(),
        "%" PRId32,
        node_id
    );
}

void Interactable::on_init(prt3::Scene & scene) {
    thread_local std::vector<prt3::NodeID> queue;

    for (prt3::NodeID child_id : scene.get_node(node_id()).children_ids()) {
        queue.push_back(child_id);
    }

    while (!queue.empty()) {
        prt3::NodeID id = queue.back();
        queue.pop_back();

        prt3::Node const & node = scene.get_node(id);

        if (scene.has_component<prt3::ColliderComponent>(id)) {
            prt3::ColliderComponent const & col =
                scene.get_component<prt3::ColliderComponent>(id);
            if (col.tag().type == prt3::ColliderType::area) {
                m_triggers.push_back(id);
            }
        }

        if (scene.has_component<prt3::ScriptSet>(id)) {
            prt3::ScriptSet const & script_set =
                scene.get_component<prt3::ScriptSet>(id);
            for (prt3::ScriptID script_id : script_set.get_all_scripts()) {
                scene.connect_signal(m_signal, scene.get_script(script_id));
            }
        }

        for (prt3::NodeID child_id : node.children_ids()) {
            queue.push_back(child_id);
        }
    }

    m_game_state = scene.get_autoload_script<GameState>();
}

void Interactable::on_update(prt3::Scene & scene, float /*delta_time*/) {
    for (prt3::NodeID id : m_triggers) {
        for (prt3::NodeID overlap : scene.get_overlaps(id)) {
            if (overlap == m_game_state->player_id() &&
                scene.get_input().get_key_down(dds::input_mapping::interact)) {
                scene.emit_signal(m_signal, nullptr);
            }
        }
    }
}
