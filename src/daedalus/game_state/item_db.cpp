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
    {
        /* item_spell_flame_pillar */
        Item & item = m_items[item_spell_fire_rock];
        item.use_duration = 500 * dds::time_scale;
        item.cooldown = 10000 * dds::time_scale;
        item.prefab_id = PrefabDB::fire_rock;
        item.object.type = ObjectDB::projectile;
        item.object.u.projectile.local_rotation_axis = glm::vec3{1.0f, 0.0f, 0.0f};
        item.object.u.projectile.rotation_speed =
            glm::radians(10.0f * dds::time_scale);
        item.object.u.projectile.angle = 0.0f;
        item.object.u.projectile.radius = 1.0f;
        item.object.u.projectile.fade_time = 250 * dds::time_scale;
        item.object.u.projectile.duration = 5000 * dds::time_scale;
        item.object.u.projectile.sustain = 2000 * dds::time_scale;
        item.object.u.projectile.velocity = 15.0f / dds::time_scale;
        item.object.u.projectile.homing_force = 0.5f;
        item.object.u.projectile.timer = 0;
        // item.object.u.projectile.hit_prefab = /* TODO */;
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

void ItemDB::use(
    prt3::Scene const & scene,
    NPCID npc_id,
    ItemID item_id,
    AnyID const & target
) {
    Item const & item = m_items[item_id];
    NPCDB const & npc_db = m_game_state.npc_db();

    MapPosition target_pos = npc_db.get_target_position(scene, target);
    switch (item.object.type) {
        case ObjectDB::area_of_effect: {
            m_game_state.object_db().add_object<AOE>(
                item.object.u.aoe,
                target_pos,
                item.prefab_id
            );
            break;
        }
        case ObjectDB::projectile: {
            Projectile projectile = item.object.u.projectile;
            projectile.target = target;

            NPC const & npc = npc_db.get_npc(npc_id);
            glm::vec3 npc_pos = npc.map_position.position;
            glm::vec3 tpos = target_pos.position;
            tpos.y += 0.5f * npc_db.get_target_height(target);
            glm::vec3 dir = tpos - npc_pos;
            if (dir != glm::vec3{0.0f}) dir = glm::normalize(dir);
            projectile.current_dir = dir;

            MapPosition spawn_pos;
            spawn_pos.position = npc_pos;
            spawn_pos.room = npc.map_position.room;
            spawn_pos.position.y += npc.collider_height +
                                    npc.model_scale +
                                    2.0f; // TODO: don't hardcode height offset

            m_game_state.object_db().add_object<Projectile>(
                projectile,
                spawn_pos,
                item.prefab_id
            );
            break;
        }
        default: {}
    }
    CooldownKey key = CooldownKey{npc_id, item_id};
    m_cooldown_timestamps[key] = m_game_state.current_time();
}
