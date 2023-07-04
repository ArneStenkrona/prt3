#ifndef PRT3_WEAPON_H
#define PRT3_WEAPON_H

#include "src/engine/scene/node.h"
#include "src/util/uuid.h"
#include "src/engine/physics/collider.h"
#include "src/engine/component/script/script.h"
#include "src/engine/scene/signal.h"

#include <unordered_set>

namespace prt3 {

class Scene;
template<typename T>
class ComponentStorage;

struct HitPacket {
    NodeID node_id;
};

class Weapon {
public:
    Weapon(
        Scene & scene,
        NodeID node_id
    );

    Weapon(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }

    bool is_active() { return m_active; }
    void set_active(Scene & scene, bool active);

    static void connect_hit_signal(Scene & scene, Script & script, NodeID id);
    static bool is_hit_signal(SignalString const & signal);

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Weapon"; }
    static constexpr UUID uuid = 13282231132589125850ull;

    static constexpr CollisionLayer WEAPON_LAYER = 1 << 8;

private:
    NodeID m_node_id;

    bool m_active;

    std::unordered_set<NodeID> m_hits;

    HitPacket m_packet;

    void remove(Scene & /*scene*/) {}

    static void update(Scene & scene, std::vector<Weapon> & components);

    friend class ComponentManager;
    friend class ComponentStorage<Weapon>;
};

} // namespace prt3

#endif // PRT3_WEAPON_H
