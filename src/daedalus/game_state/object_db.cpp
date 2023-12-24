#include "object_db.h"

#include "src/daedalus/game_state/game_state.h"

using namespace dds;

void AOE::update(
    prt3::Scene & scene,
    ObjectDB & db,
    AOE * entries,
    ObjectID const * index_to_id,
    size_t n_entries,
    std::vector<ObjectID> & remove_list
) {
    for (uint32_t index = 0; index < n_entries; ++index) {
        auto & entry = entries[index];
        ObjectID id = index_to_id[index];

        entry.timer += dds::ms_per_frame;

        prt3::NodeID node_id = db.get_loaded_object(id);
        if (node_id == prt3::NO_NODE) {
            if (entry.timer > entry.fade_time +
                              entry.duration +
                              entry.sustain) {
                remove_list.push_back(id);
            }
            continue;
        }

        /* update loaded object */
        prt3::Decal & decal = scene.get_component<prt3::Decal>(node_id);
        auto & ps = scene.get_component<prt3::ParticleSystem>(node_id);

        float alpha;
        if (entry.timer < entry.fade_time) {
            alpha = float(entry.timer) / float(entry.fade_time);
            ps.parameters().active = false;
        } else if (entry.timer < entry.fade_time + entry.duration) {
            alpha = 1.0f;
            ps.parameters().active = true;
        } else {
            ps.parameters().emission_rate = 0.0f;
            alpha = 1.0f - glm::min(
                float(entry.timer - (entry.fade_time + entry.duration)) /
                float(entry.fade_time),
                1.0f
            );
        }

        decal.color().a = alpha;

        if (entry.timer > entry.fade_time + entry.duration &&
            ps.active_particles() == 0) {
            remove_list.push_back(id);
        }
    }
}

TimeMS ObjectDB::current_time() {
    return m_game_state.current_time();
}

ObjectDB::ObjectDB(GameState & game_state)
 : m_game_state{game_state} {}


void ObjectDB::update(prt3::Scene & scene) {
    load_objects(scene);
    unload_objects(scene);
    thread_local std::vector<ObjectID> remove_list;
    update_objects(scene, remove_list);
}

void ObjectDB::load_objects(prt3::Scene & scene) {
    for (ObjectID id = 0; id < m_objects.size(); ++id) {
        Object const & object = m_objects[id];
        if (object.position.room == m_game_state.current_room()) {
            if (m_loaded_objects.find(id) == m_loaded_objects.end()) {
                prt3::NodeID node_id = m_game_state.prefab_db()
                    .get(object.prefab_id)
                    .instantiate(scene, scene.root_id());
                m_loaded_objects[id] = node_id;
                scene.get_node(node_id).set_global_position(
                    scene,
                    object.position.position
                );

                if (scene.has_component<prt3::ParticleSystem>(node_id)) {
                    prt3::ParticleSystem & ps =
                        scene.get_component<prt3::ParticleSystem>(node_id);
                    TimeMS ms = m_game_state.current_time() - object.timestamp;
                    if (object.type == ObjectType::area_of_effect) {
                        AOE const & aoe = m_object_database.get_entry<AOE>(id);
                        ms -= aoe.fade_time;
                    }

                    if (ms > 0) {
                        TimeMS n_frames = ms / ms_per_frame;
                        float time = n_frames * frame_dt;
                        time = glm::min(time, ps.parameters().lifetime.max);
                        ps.parameters().active = true;
                        ps.parameters().prewarm = time;
                    } else {
                        ps.parameters().active = false;
                    }
                }
            }
        }
    }
}

void ObjectDB::unload_objects(prt3::Scene & scene) {
    for (auto it = m_loaded_objects.begin(); it != m_loaded_objects.end();) {
        ObjectID id = it->first;
        Object & object = m_objects[id];
        if (object.position.room != m_game_state.current_room()) {
            /* unload object */
            prt3::NodeID node_id = it->second;
            scene.remove_node(node_id);

            m_loaded_objects.erase(it++);
        } else {
            ++it;
        }
    }
}
