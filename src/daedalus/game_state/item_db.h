#ifndef DDS_ITEM_DB_H
#define DDS_ITEM_DB_H

#include "src/daedalus/game_state/game_time.h"
#include "src/daedalus/game_state/id.h"
#include "src/daedalus/game_state/object_db.h"
#include "src/daedalus/game_state/prefab_db.h"

#include "src/util/hash_util.h"

#include <array>
#include <functional>
#include <unordered_map>

namespace prt3 {
class Scene;
} // namespace prt3

namespace dds {

struct Item {
    TimeMS use_duration; // estimate during simulation
    TimeMS cooldown;
    PrefabDB::PrefabID prefab_id;
    ObjectDB::ObjectUnion object;
};

enum ItemID : DDSID {
    item_spell_flame_pillar,
    item_spell_fire_rock,
    n_item_ids,
    item_none = n_item_ids
};

struct CooldownKey {
    NPCID npc_id;
    ItemID item_id;

    bool operator ==(CooldownKey const & other) const {
        return npc_id == other.npc_id && item_id == other.item_id;
    }

    bool operator !=(CooldownKey const & other) const {
        return !(*this == other);
    }
};

} // namespace dds

template<> struct std::hash<dds::CooldownKey> {
    size_t operator()(dds::CooldownKey const & k) const {
        size_t hash = k.npc_id;
        prt3::hash_combine(hash, k.item_id);
        return hash;
    }
};

namespace dds {

class GameState;

class ItemDB {
public:
    ItemDB(GameState & game_state);

    void update();

    Item const & get(ItemID id) const {
        return m_items[id];
    }

    inline static bool is_spell(ItemID id) {
        switch(id) {
            case item_spell_flame_pillar:
            case item_spell_fire_rock:
                return true;
            default: {}
        }
        return false;
    }

    void use(
        prt3::Scene const & scene,
        NPCID npc_id,
        ItemID item_id,
        AnyID const & target
    );

    inline bool ready_to_use(NPCID npc_id, ItemID item_id) const {
        auto it = m_cooldown_timestamps.find(CooldownKey{npc_id, item_id});
        return it == m_cooldown_timestamps.end();
    }

private:
    std::array<Item, n_item_ids> m_items;
    std::unordered_map<CooldownKey, TimeMS> m_cooldown_timestamps;

    GameState & m_game_state;
};

} // namespace dds

#endif // DDS_ITEM_DB_H
