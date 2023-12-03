#include "weapon.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"
#include "src/engine/component/collider_component.h"

#include <charconv>

using namespace prt3;

Weapon::Weapon(
    Scene &,
    NodeID node_id
)
 : m_node_id{node_id} {
    m_packet.node_id = node_id;
 }

Weapon::Weapon(
    Scene &,
    NodeID node_id,
    std::istream &
)
 : m_node_id{node_id} {
    m_packet.node_id = node_id;
 }

void Weapon::serialize(
    std::ostream &,
    Scene const &
) const {}

void Weapon::set_active(Scene & scene, bool active) {
    if (active == m_active) return;
    m_active = active;
    if (!active) {
        m_hits.clear();
    }

    ColliderComponent & col =
        scene.get_component<ColliderComponent>(node_id());

    PhysicsSystem & sys = scene.physics_system();
    CollisionLayer mask = sys.get_collision_mask(col.tag());
    CollisionLayer new_mask = m_active ?
        mask | WEAPON_LAYER : mask & WEAPON_LAYER;

    if (mask != new_mask) {
        sys.set_collision_mask(col.tag(), new_mask);
    }
}

#define HIT_SIGNAL_BASE "<hit>"

void Weapon::connect_hit_signal(Scene & scene, Script & script, NodeID id) {
    alignas(NodeID) SignalString signal = HIT_SIGNAL_BASE;
    char * id_ptr = signal.data() + sizeof(HIT_SIGNAL_BASE) - 1;

    std::to_chars(id_ptr, id_ptr + 10, id);

    scene.connect_signal(signal, &script);
}

bool Weapon::is_hit_signal(SignalString const & signal) {
    return strncmp(
        signal.data(),
        HIT_SIGNAL_BASE,
        sizeof(HIT_SIGNAL_BASE) - 1
    );
}

void Weapon::update(
    Scene & scene,
    float /*delta_time*/,
    std::vector<Weapon> & components
) {
    SignalString signal = HIT_SIGNAL_BASE;
    char * id_ptr = signal.data() + sizeof(HIT_SIGNAL_BASE) - 1;

    for (Weapon & weapon : components) {
        if (!weapon.m_active) continue;

        std::vector<NodeID> const & overlaps =
            scene.get_overlaps(weapon.node_id());

        for (NodeID id : overlaps) {
            if (weapon.m_hits.find(id) == weapon.m_hits.end()) {
                std::to_chars(id_ptr, id_ptr + 10, id);
                scene.emit_signal(signal, &weapon.m_packet);
            }
        }

        weapon.m_hits.insert(overlaps.begin(), overlaps.end());
    }
}
