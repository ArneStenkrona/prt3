#ifndef DDS_MAP_H
#define DDS_MAP_H

#include "src/engine/component/transform.h"
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

struct ParsingContext;

} // namespace dds

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

    MapPosition get_map_destination(MapPathID id)
    { return m_map_path_cache.access(id)->path.back().position; }

    bool advance_map_path(
        MapPathID id,
        glm::vec3 position,
        float delta,
        MapPosition & out_pos,
        glm::vec3 & out_dir
    );

    static std::string room_to_scene_path(RoomID room_id);
    static RoomID scene_to_room(prt3::Scene const & scene);


    uint32_t local_to_global_door_id(RoomID room, uint32_t door_id) const
    { return m_local_ids.at(std::pair<RoomID, uint32_t>(room, door_id)); }
    uint32_t get_door_destination_id(uint32_t door_id) const
    { return static_cast<uint32_t>(m_doors[door_id].dest); }

    glm::vec3 get_door_entry_position(uint32_t door_id) const;

private:
      struct MapRoom {
        enum RoomType {
            indoors,
            outdoors
        };

        prt3::SubVec doors;
        RoomType type;
    };

    struct MapDoor {
        prt3::Transform shape;
        glm::vec3 entry_offset;
        int32_t dest;
        uint32_t local_id;
        MapPosition position;
    };

    std::vector<MapRoom> m_rooms;
    std::vector<MapPosition> m_locations;
    std::unordered_map<std::string, LocationID> m_location_ids;

    prt3::NavMeshID m_nav_mesh_id = prt3::NO_NAV_MESH;

    std::vector<MapDoor> m_doors;

    struct pairhash {
        size_t operator()(std::pair<dds::RoomID, uint32_t> const & p) const {
            size_t hash = static_cast<size_t>(p.first);
            prt3::hash_combine(hash, p.second);
            return hash;
        }
    };

    std::unordered_map<std::pair<RoomID, uint32_t>, uint32_t, pairhash> m_local_ids;

    struct NavPath {
        float length;
        std::vector<glm::vec3> path;
    };

    struct MapPathEntry {
        MapPosition position;
        float door_intersection;
        float accumulated_distance;
    };

    struct MapPath {
        std::vector<MapPathEntry> path;
        float length;
        uint32_t curr_ind;
    };

    static constexpr size_t NumMapPathCacheEntry = 128;
    prt3::LRUCache<MapPathID, MapPath, NumMapPathCacheEntry> m_map_path_cache;
    MapPathID m_next_map_path_id = 0;

    prt3::NavigationSystem m_navigation_system;

    Map() {}

    bool intersects_door(
        RoomID room_id,
        glm::vec3 a,
        glm::vec3 b,
        float & t,
        uint32_t & door_id
    );

    bool get_map_path(
        MapPosition from,
        MapPosition to,
        std::vector<MapPathEntry> & path
    );

    NavPath const * get_nav_path(
        glm::vec3 from,
        glm::vec3 to
    );

    float get_nav_path_length(
        glm::vec3 from,
        glm::vec3 to
    );

    static prt3::NodeID map_node_to_new_scene_node(
        ParsingContext & ctx,
        uint32_t map_node_index,
        uint32_t room_index,
        uint32_t node_index,
        prt3::Transform global_tform,
        char const * name,
        prt3::Scene & scene
    );

    static bool parse_door(
        ParsingContext & ctx,
        uint32_t map_node_index,
        uint32_t room_index,
        uint32_t node_index,
        prt3::Transform global_tform,
        prt3::Scene & scene
    );

    static bool parse_location(
        ParsingContext & ctx,
        uint32_t map_node_index,
        uint32_t room_index,
        uint32_t node_index,
        prt3::Transform global_tform
    );

    static bool parse_object(
        ParsingContext & ctx,
        uint32_t map_node_index,
        uint32_t room_index,
        uint32_t node_index,
        prt3::Transform global_tform,
        prt3::Scene & scene
    );

    static bool parse_interactable(
        ParsingContext & ctx,
        uint32_t map_node_index,
        uint32_t room_index,
        uint32_t node_index,
        prt3::Transform global_tform,
        prt3::Scene & scene
    );

    static bool parse_slide(
        ParsingContext & ctx,
        uint32_t map_node_index,
        uint32_t room_index,
        uint32_t node_index,
        prt3::Transform global_tform,
        prt3::Scene & scene
    );

    static bool parse_collider_trigger_common(
        ParsingContext & ctx,
        uint32_t map_node_index,
        uint32_t room_index,
        uint32_t node_index,
        prt3::Transform global_tform,
        bool is_collider,
        prt3::Scene & scene
    );

    static bool parse_collider(
        ParsingContext & ctx,
        uint32_t map_node_index,
        uint32_t room_index,
        uint32_t node_index,
        prt3::Transform global_tform,
        prt3::Scene & scene
    ) {
        return parse_collider_trigger_common(
            ctx, map_node_index, room_index, node_index, global_tform, true, scene
        );
    }

    static bool parse_trigger(
        ParsingContext & ctx,
        uint32_t map_node_index,
        uint32_t room_index,
        uint32_t node_index,
        prt3::Transform global_tform,
        prt3::Scene & scene
    ) {
        return parse_collider_trigger_common(
            ctx, map_node_index, room_index, node_index, global_tform, false, scene
        );
    }

    static uint32_t copy_mesh(
        ParsingContext & ctx,
        uint32_t src_mesh_index,
        prt3::Model & dest_model,
        uint32_t node_index,
        std::unordered_map<uint32_t, uint32_t> & material_map
    );

    static void generate_nav_mesh(
        prt3::Context & prt3_context,
        ParsingContext & ctx,
        prt3::Model const & model
    );

    friend struct ParsingContext;
};

struct ParsingContext {
    Map map;
    prt3::Model * map_model;
    std::vector<prt3::Model> models;
    std::unordered_map<uint32_t, uint32_t> num_to_room_node;
    std::unordered_map<uint32_t, uint32_t> num_to_room_index;
    std::vector<std::unordered_map<uint32_t, uint32_t> > num_to_door;
    std::unordered_map<uint32_t, uint32_t> door_num_to_dest_room;
    std::unordered_map<int32_t, prt3::NodeID> model_node_to_scene_node;
    std::vector<prt3::Model> object_models;
    std::unordered_map<uint32_t, std::unordered_map<prt3::NodeID, uint32_t> > object_meshes;
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t> > material_maps;
};

} // namespace dds

#endif // PRT3_MAP_H
