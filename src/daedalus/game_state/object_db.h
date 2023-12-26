#ifndef DDS_OBJECT_DB_H
#define DDS_OBJECT_DB_H

#include "src/daedalus/game_state/game_time.h"
#include "src/daedalus/game_state/id.h"
#include "src/daedalus/game_state/prefab_db.h"
#include "src/daedalus/map/map.h"
#include "src/engine/component/particle_system.h"
#include "src/engine/scene/scene.h"
#include "src/util/database/database.h"

namespace dds {

class ObjectDB;

struct AOE {
    float radius;
    float height;
    TimeMS fade_time;
    TimeMS duration;
    TimeMS sustain;
    TimeMS timer;

    static void update(
        prt3::Scene & scene,
        ObjectDB & db,
        AOE * entries,
        ObjectID const * index_to_id,
        size_t n_entries,
        std::vector<ObjectID> & remove_list
    );
};

class GameState;

class ObjectDB {
public:
    using ObjectDatabase = prt3::Database<
        /* id type */
        ObjectID,
        /* data types */
        AOE
    >;

    enum ObjectType {
        area_of_effect = ObjectDatabase::get_table_index<AOE>(),
    };

    template<typename T>
    inline static ObjectType object_to_enum() {
        return static_cast<ObjectType>(ObjectDatabase::get_table_index<T>());
    }

    struct Object {
        MapPosition position;
        PrefabDB::PrefabID prefab_id;
        ObjectType type;
        TimeMS timestamp;

        inline bool exists() const
        { return position.room != NO_ROOM; }
    };

    struct ObjectUnion {
        union U {
            U() : aoe{} {}
            AOE aoe;
        } u;
        ObjectType type;
    };

    ObjectDB(GameState & game_state);

    Object & get_object(ObjectID id) {
        return m_objects[id];
    }

    Object const & get_object(ObjectID id) const {
        return m_objects[id];
    }

    template<typename T>
    inline void add_object(
        T const & obj,
        MapPosition const & position,
        PrefabDB::PrefabID prefab_id
    ) {
        ObjectID id;
        if (!m_free_ids.empty()) {
            id = m_free_ids.back();
            m_free_ids.pop_back();
        } else {
            id = m_objects.size();
            m_objects.push_back({});
        }
        m_objects[id].position = position;
        m_objects[id].prefab_id = prefab_id;
        m_objects[id].type = object_to_enum<T>();
        m_objects[id].timestamp = current_time();

        m_object_database.add_entry<T>(id, obj);
    }

    template<typename T>
    inline void remove_object(ObjectID id) {
        m_object_database.remove_entry<T>(id);
        m_free_ids.push_back(id);
        m_objects[id].position.room = NO_ROOM;
    }

    TimeMS current_time();

    void on_scene_start();

    void update(prt3::Scene & scene);

    prt3::NodeID get_loaded_object(ObjectID id) const {
        auto it = m_loaded_objects.find(id);
        if (it == m_loaded_objects.end()) {
            return prt3::NO_NODE;
        }
        return it->second;
    }

private:
    ObjectDatabase m_object_database;

    std::vector<Object> m_objects;
    std::vector<ObjectID> m_free_ids;

    std::unordered_map<ObjectID, prt3::NodeID> m_loaded_objects;

    GameState & m_game_state;

    template<size_t I = 0>
    inline void update_objects(
        prt3::Scene & scene,
        std::vector<ObjectID> & remove_list
    ) {
        auto & table = std::get<I>(m_object_database.get_tables());

        std::remove_reference<decltype(table)>::type::data_type::update(
            scene,
            *this,
            table.get_entries(),
            table.index_map(),
            table.num_entries(),
            remove_list
        );

        for (ObjectID id : remove_list) {
            table.remove_entry(id);
            m_free_ids.push_back(id);
            m_objects[id].position.room = NO_ROOM;
        }
        remove_list.clear();

        if constexpr(I+1 != ObjectDatabase::n_data_types)
            update_objects<I+1>(scene, remove_list);
    }

    bool load_aoe(prt3::Scene & scene, ObjectID id);

    void load_objects(prt3::Scene & scene);
    void unload_objects(prt3::Scene & scene);
};

} // namespace dds

#endif // DDS_OBJECT_DB_H
