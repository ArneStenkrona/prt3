#include "object_db.h"

#include "src/daedalus/game_state/game_state.h"

#include "src/util/geometry_util.h"

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
            alpha = entry.fade_time == 0 ? 0.0f :
                1.0f - glm::min(
                    float(entry.timer - (entry.fade_time + entry.duration)) /
                    float(entry.fade_time),
                    1.0f
                );
        }

        if (scene.has_component<prt3::Decal>(node_id)) {
            prt3::Decal & decal = scene.get_component<prt3::Decal>(node_id);
            decal.color().a = alpha;
        }

        if (entry.timer > entry.fade_time + entry.duration &&
            ps.active_particles() == 0) {
            remove_list.push_back(id);
        }
    }
}

void Projectile::update(
    prt3::Scene & scene,
    ObjectDB & db,
    Projectile * entries,
    ObjectID const * index_to_id,
    size_t n_entries,
    std::vector<ObjectID> & remove_list
) {
    for (uint32_t index = 0; index < n_entries; ++index) {
        auto & entry = entries[index];
        ObjectID id = index_to_id[index];

        entry.timer += dds::ms_per_frame;

        ObjectDB::Object & object = db.get_object(id);
        glm::vec3 & pos = object.position.position;

        if (entry.homing_force > 0.0f) {
            NPCDB const & npc_db = db.game_state().npc_db();
            glm::vec3 target_pos = npc_db.get_target_position(
                scene,
                entry.target
            ).position;
            target_pos.y += 0.5f * npc_db.get_target_height(entry.target);

            glm::vec3 target_dir = target_pos - pos;
            entry.current_dir = GameState::smooth_change_dir(
                entry.current_dir,
                target_dir,
                entry.homing_force,
                dds::frame_dt_over_time_scale
            );
        }

        pos += entry.velocity * entry.current_dir * dds::frame_dt;

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
        prt3::Node & node = scene.get_node(node_id);

        auto & ps = scene.get_component<prt3::ParticleSystem>(node_id);

        float alpha;
        if (entry.timer < entry.fade_time) {
            alpha = float(entry.timer) / float(entry.fade_time);
        } else if (entry.timer < entry.fade_time + entry.duration) {
            alpha = 1.0f;
        } else {
            ps.parameters().emission_rate = 0.0f;
            alpha = 1.0f - glm::min(
                float(entry.timer - (entry.fade_time + entry.duration)) /
                float(entry.fade_time),
                1.0f
            );
        }

        prt3::MaterialComponent & mat =
            scene.get_component<prt3::MaterialComponent>(node_id);
        mat.material_override().tint_active = alpha < 1.0f;
        mat.material_override().tint.a = alpha;

        prt3::Transform & tform = node.local_transform();
        auto rot = glm::quat(glm::vec3{0.0f, 0.0f, 1.0f}, entry.current_dir);
        tform.rotation = rot;

        if (entry.rotation_speed != 0.0f) {
            glm::vec3 rot_axis = tform.rotation * entry.local_rotation_axis;
            entry.angle += entry.rotation_speed * dds::frame_dt_over_time_scale;
            tform.rotation = tform.rotation * glm::angleAxis(
                entry.angle,
                rot_axis
            );
        }

        if (entry.timer > entry.fade_time + entry.duration &&
            ps.active_particles() == 0) {
            remove_list.push_back(id);
        }

        if (entry.timer > entry.fade_time &&
            entry.timer < entry.fade_time + entry.duration &&
            !scene.get_overlaps(node_id).empty()) {
            entry.timer = entry.fade_time + entry.duration;
            entry.velocity = 0.0f;
            entry.rotation_speed = 0.0f;
            auto & col = scene.get_component<prt3::ColliderComponent>(node_id);
            col.set_mask(scene, 0);

            AOE aoe;
            aoe.radius = 2.0f;
            aoe.height = 0.0f;
            aoe.fade_time = 0;
            aoe.duration = 200 * dds::time_scale;
            aoe.sustain = 750 * dds::time_scale;
            aoe.timer = 0;

            MapPosition map_pos = object.position;
            // This invalidates object reference
            db.add_object<AOE>(
                aoe,
                map_pos,
                entry.hit_prefab
            );
        }
    }
}

TimeMS ObjectDB::current_time() {
    return m_game_state.current_time();
}

ObjectDB::ObjectDB(GameState & game_state)
 : m_game_state{game_state} {}

void ObjectDB::on_scene_start() {
    m_loaded_objects.clear();
}

void ObjectDB::update(prt3::Scene & scene) {
    thread_local std::vector<ObjectID> remove_list;
    update_objects(scene, remove_list);
    move_objects_between_rooms();
    load_objects(scene);
    unload_objects(scene);
    update_positions(scene);
}

void ObjectDB::update_positions(prt3::Scene & scene) {
    for (ObjectID i = 0; i < m_objects.size(); ++i) {
        if (!m_objects[i].exists()) continue;
        m_position_history[i] = m_objects[i].position.position;
    }

    for (auto & pair : m_loaded_objects) {
        scene.get_node(pair.second).local_transform().position =
            m_objects[pair.first].position.position;
    }
}


void ObjectDB::move_objects_between_rooms() {
    thread_local std::vector<prt3::ColliderID> ids;
    ids.clear();

    Map & map = m_game_state.map();

    struct IntersectRes {
        ObjectID object_id;
        int32_t num;
    };

    thread_local std::vector<IntersectRes> res;
    res.clear();

    for (ObjectID i = 0; i < m_objects.size(); ++i) {
        if (!m_objects[i].exists()) continue;

        Object & object = m_objects[i];

        if (object.position.position == m_position_history[i]) continue;

        glm::vec3 dir = object.position.position - m_position_history[i];
        float length = glm::length(dir);
        dir = glm::normalize(dir);

        size_t before = ids.size();
        map.query_doors_raycast(
            object.position.room,
            m_position_history[i],
            dir,
            length,
            ids
        );

        if (ids.size() > before) {
            int32_t n_ids = static_cast<int32_t>(ids.size() - before);
            res.push_back({i, n_ids});
        }
    }

    if (ids.empty()) {
        return;
    }

    std::vector<glm::vec3> const & geom = map.door_geometry();

    uint32_t id_index = 0;
    for (IntersectRes & r : res) {
        Object & object = m_objects[r.object_id];
        glm::vec3 o = m_position_history[r.object_id];
        glm::vec3 v = object.position.position - o;

        uint32_t n_ids = r.num;
        r.num = -1;
        for (uint32_t i = 0; i < n_ids; ++i) {
            uint32_t door_id = ids[id_index + i];
            glm::vec3 const * gs = &geom[map.door_to_vertex_index(door_id)];
            if (prt3::triangle_ray_intersect(o, v, gs[0], gs[1], gs[3]) ||
                prt3::triangle_ray_intersect(o, v, gs[2], gs[1], gs[3])) {
                r.num = door_id;
                break;
            }
        }

        id_index += n_ids;
    }

    for (IntersectRes & r : res) {
        if (r.num == -1) continue;
        Object & object = m_objects[r.object_id];
        glm::vec3 p_door = m_game_state.get_door_local_position(
            r.num,
            object.position.position
        );
        uint32_t dest_door = map.get_door_destination_id(r.num);
        glm::vec3 new_pos = p_door + map.get_door_entry_position(dest_door);
        object.position.position = new_pos;
        object.position.room = map.door_to_room(dest_door);
    }
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
                        ps.parameters().active = ms > 0;
                    } else if (object.type == ObjectType::projectile) {
                        Projectile const & projectile =
                            m_object_database.get_entry<Projectile>(id);
                        ms -= projectile.fade_time;
                        ps.parameters().active = true;
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
