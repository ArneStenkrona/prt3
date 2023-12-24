#include "item_db.h"

#include "src/daedalus/game_state/game_state.h"

using namespace dds;

ItemDB::ItemDB(GameState & game_state)
 : m_game_state{game_state} {
    {
        /* item_spell_flame_pillar */
        Item & item = m_items[item_spell_flame_pillar];
        item.use_duration = 500 * dds::time_scale;
        item.cooldown = 5000 * dds::time_scale;
        item.prefab_id = PrefabDB::flame_pillar;
        item.object.type = ObjectDB::area_of_effect;
        item.object.u.aoe.radius = 1.0f;
        item.object.u.aoe.height = 4.0f;
        item.object.u.aoe.fade_time = 250 * dds::time_scale;
        item.object.u.aoe.duration = 2000 * dds::time_scale;
        item.object.u.aoe.sustain = 2000 * dds::time_scale;
        item.object.u.aoe.timer = 0;
    }
}

void ItemDB::update() {
    for (auto it = m_cooldown_timestamps.begin();
         it != m_cooldown_timestamps.end();) {
        TimeMS deadline = it->second + m_items[it->first.item_id].cooldown;
        if (deadline <= m_game_state.current_time()) {
            m_cooldown_timestamps.erase(it++);
        } else {
            ++it;
        }
    }
}

void ItemDB::use(NPCID npc_id, ItemID item_id, MapPosition const & position) {
    Item const & item = m_items[item_id];
    switch (item.object.type) {
        case ObjectDB::area_of_effect: {
            m_game_state.object_db().add_object<AOE>(
                item.object.u.aoe,
                position,
                item.prefab_id
            );
            break;
        }
        default: {}
    }
    CooldownKey key = CooldownKey{npc_id, item_id};
    m_cooldown_timestamps[key] = m_game_state.current_time();
}
