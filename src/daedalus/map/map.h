#ifndef DDS_MAP_H
#define DDS_MAP_H

#include "src/engine/navigation/navigation_system.h"
#include "src/util/hash_util.h"
#include "src/util/lru_cache.h"
#include "src/util/sub_vec.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <vector>

namespace dds {

typedef uint32_t RoomID;
typedef int32_t LocationID;
constexpr LocationID NO_LOCATION = -1;

typedef int64_t MapPathID;
constexpr MapPathID NO_MAP_PATH = -1;

struct MapPosition {
    glm::vec3 position;
    RoomID room;
};

struct NavPathKey {
    RoomID room;
    glm::vec3 from;
    glm::vec3 to;

    bool operator ==(NavPathKey const & other) const {
        return room == other.room && from == other.from && to == other.to;
    }

    bool operator !=(NavPathKey const & other) const {
        return !(*this == other);
    }

};

} // namespace std

namespace std {
    template<>
    struct hash<dds::NavPathKey> {
        size_t operator()(dds::NavPathKey const & k) const {
            size_t hash = static_cast<size_t>(k.room);
            prt3::hash_combine(hash, k.from);
            prt3::hash_combine(hash, k.to);
            return hash;
        }
    };
}

namespace dds {

class Map {
public:
    Map(char const * path);

    static Map parse_map_from_model(char const * path);

    void serialize(std::ofstream & out);
    void deserialize(std::ifstream & in);

    bool has_map_path(MapPathID id) const
    { return m_map_path_cache.has_key(id); }

    MapPathID query_map_path(MapPosition origin, MapPosition destination);

    float get_map_path_length(MapPathID id)
    { return m_map_path_cache.access(id)->length; }

    MapPosition interpolate_map_path(MapPathID id, float t);

    static std::string room_to_scene_path(RoomID room_id);

private:
    struct MapRoom {
        prt3::SubVec doors;
        prt3::NavMeshID nav_mesh_id;
    };

    std::vector<MapRoom> m_rooms;
    std::vector<MapPosition> m_locations;
    std::unordered_map<std::string, LocationID> m_location_ids;

    struct MapDoor {
        MapPosition position;
        uint32_t dest;
    };

    std::vector<MapDoor> m_doors;

    struct NavPath {
        float length;
        std::vector<glm::vec3> path;
    };

    static constexpr size_t NumNavPathCacheEntry = 128;
    prt3::LRUCache<NavPathKey, NavPath, NumNavPathCacheEntry> m_nav_mesh_path_cache;

    static constexpr size_t NumNavPathLengthCacheEntry = 8192;
    prt3::LRUCache<NavPathKey, float, NumNavPathLengthCacheEntry> m_nav_mesh_path_length_cache;

    struct MapPathEntry {
        MapPosition position;
        float accumulated_distance;
    };

    struct MapPath {
        std::vector<MapPathEntry> path;
        float length;
    };

    static constexpr size_t NumMapPathCacheEntry = 128;
    prt3::LRUCache<MapPathID, MapPath, NumMapPathCacheEntry> m_map_path_cache;
    MapPathID m_next_map_path_id = 0;



    prt3::NavigationSystem m_navigation_system;

    Map() {}

    bool get_map_path(
        MapPosition from,
        MapPosition to,
        std::vector<MapPathEntry> & path
    );

    NavPath const * get_nav_path(
        RoomID room_id,
        glm::vec3 from,
        glm::vec3 to
    );

    float get_nav_path_length(
        RoomID room_id,
        glm::vec3 from,
        glm::vec3 to
    );
};

} // namespace dds

#endif // PRT3_MAP_H
