#include "map.h"

#include "src/engine/rendering/model.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/collider_component.h"
#include "src/engine/component/door.h"
#include "src/engine/core/backend_type.h"
#include "src/engine/core/context.h"
#include "src/util/serialization_util.h"
#include "src/util/file_util.h"
#include "src/util/log.h"
#include "src/util/sub_vec.h"

#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace dds;

Map::Map(char const * path) {
    std::ifstream in{path, std::ios::binary};
    if(in.fail()){
        return;
    }
    deserialize(in);
}

#define TOK_ROOM "room"
#define TOK_DOOR "door"
#define TOK_LOCATION "loc"

inline bool check_tok(char const * tok, char const * str) {
    return strncmp(tok, str, strlen(tok)) == 0;
}

Map Map::parse_map_from_model(char const * path) {
    Map map;

    prt3::Context context{prt3::BackendType::dummy};

    prt3::Model map_model{path};
    auto const & nodes = map_model.nodes();
    auto const & map_vertices = map_model.vertex_buffer();
    auto const & map_indices = map_model.index_buffer();

    std::unordered_map<uint32_t, uint32_t> num_to_room_node;
    std::unordered_map<uint32_t, uint32_t> num_to_room_index;
    std::unordered_map<uint32_t, uint32_t> num_to_door;

    uint32_t index = 0;
    uint32_t n_rooms = 0;
    for (prt3::Model::Node const & node : nodes) {
        char const * name = node.name.c_str();

        if (check_tok(TOK_ROOM, name)) {
            uint32_t num;
            sscanf(name + strlen(TOK_ROOM), "%" SCNu32, &num);
            num_to_room_node[num] = index;
            num_to_room_index[num] = n_rooms;
            ++n_rooms;
        }
        ++index;
    }
    map.m_rooms.resize(n_rooms);

    std::unordered_map<uint32_t, uint32_t> material_map;

    std::vector<prt3::Model> models;
    models.resize(num_to_room_node.size());

    struct QueueNode {
        prt3::Transform inherited;
        uint32_t index;
        int32_t parent;
    };

    std::vector<QueueNode> node_queue;
    for (auto const & pair : num_to_room_node) {
        uint32_t room_index = num_to_room_index.at(pair.first);
        MapRoom & room = map.m_rooms[room_index];
        room.doors.start_index = map.m_doors.size();

        prt3::Scene room_scene{context};
        room_scene.ambient_light() = glm::vec3{0.7};

        prt3::Model::Node const & room_node = nodes[pair.second];
        for (auto index : room_node.child_indices) {
            node_queue.push_back({
                prt3::Transform{},
                index,
                -1
            });
        }

        material_map.clear();

        prt3::Model & model = models[room_index];
        while (!node_queue.empty()) {
            QueueNode qn = node_queue.back();
            uint32_t node_index = qn.index;
            node_queue.pop_back();
            prt3::Model::Node const & node = nodes[node_index];

            int32_t model_node_index = model.nodes().size();

            model.nodes().push_back({});
            prt3::Model::Node & room_node = model.nodes().back();
            room_node.parent_index = qn.parent;
            room_node.transform = node.transform;
            room_node.inherited_transform = qn.inherited;
            room_node.name = node.name;

            if (qn.parent != -1) {
                model.nodes()[qn.parent].child_indices
                                        .push_back(model_node_index);
            }

            prt3::Transform global_tform = prt3::Transform::compose(
                qn.inherited,
                room_node.transform
            );

            for (auto child_index : node.child_indices) {
                node_queue.push_back({
                    global_tform,
                    child_index,
                    model_node_index
                });
            }

            if (check_tok(TOK_DOOR, node.name.c_str())) {
                uint32_t door_id;
                uint32_t dest_room;
                uint32_t dest_door;
                sscanf(
                    node.name.c_str() + strlen(TOK_DOOR),
                    "%" SCNu32 "_%" SCNu32 "_%" SCNu32,
                    &door_id,
                    &dest_room,
                    &dest_door
                );

                MapDoor map_door;
                map_door.position.room = room_index;
                map_door.position.position = global_tform.position;
                map_door.dest = dest_door; // rename later
                num_to_door[map.m_doors.size()] = door_id;
                map.m_doors.emplace_back(map_door);
                ++room.doors.num_indices;

                prt3::NodeID id = room_scene.add_node_to_root(
                (std::string{"door"} + std::to_string(door_id)).c_str()
                );
                room_scene.add_component<prt3::Door>(id);
                prt3::Node & node = room_scene.get_node(id);
                node.set_global_transform(room_scene, global_tform);

                prt3::Door & door_comp = room_scene.get_component<prt3::Door>(id);

                door_comp.id() = door_id;

                std::string dest_scene_path = std::string{"assets/scenes/map/room"} +
                                std::to_string(num_to_room_node.at(dest_room)) +
                                ".prt3";

                door_comp.destination_scene_path() = dest_scene_path.c_str();
                door_comp.destination_id() = dest_door;

                glm::vec3 door_depth = glm::vec3{
                    global_tform.to_matrix() *
                    glm::vec4{0.0f, 0.0f, 1.0f, 0.0f}
                };

                door_comp.entry_offset() = global_tform.get_front() *
                    (0.5f * door_depth.z + 2.0f);

                prt3::Box box{};
                box.dimensions = glm::vec3{1.0f};
                box.center.x = box.dimensions.x / 2.0f;
                box.center.y = box.dimensions.y / 2.0f;
                box.center.z = -box.dimensions.z / 2.0f;

                room_scene.add_component<prt3::ColliderComponent>(
                    id,
                    prt3::ColliderType::area,
                    box
                );

                prt3::CollisionLayer layer_and_mask = 1 << 15;
                prt3::ColliderComponent & col =
                    room_scene.get_component<prt3::ColliderComponent>(id);
                col.set_layer(room_scene, layer_and_mask);
                col.set_mask(room_scene, layer_and_mask);
            }

            if (check_tok(TOK_LOCATION, node.name.c_str())) {
                char const * name = node.name.c_str() + strlen(TOK_LOCATION);
                MapPosition map_pos;
                map_pos.room = room_index;
                map_pos.position = global_tform.position;
                map.m_location_ids[name] =
                    static_cast<LocationID>(map.m_locations.size());
                map.m_locations.push_back(map_pos);
            }

            if (node.mesh_index == -1) continue;
            /* Mesh exists */
            room_node.mesh_index = model.meshes().size();

            prt3::Model::Mesh const & map_mesh =
                map_model.meshes()[node.mesh_index];

            if (material_map.find(map_mesh.material_index) ==
                material_map.end()) {
                material_map[map_mesh.material_index] =
                    model.materials().size();
                model.materials().push_back(
                    map_model.materials()[map_mesh.material_index]
                );
            }

            model.meshes().push_back({});
            prt3::Model::Mesh & room_mesh = model.meshes().back();
            room_mesh.start_index = model.index_buffer().size();
            room_mesh.num_indices = map_mesh.num_indices;
            room_mesh.start_bone = -1;
            room_mesh.num_bones = -1;
            room_mesh.material_index =
                material_map.at(map_mesh.material_index);
            room_mesh.node_index = model_node_index;
            room_mesh.name = map_mesh.name;

            uint32_t index = map_mesh.start_index;
            uint32_t lowest = glm::max(map_vertices.size(), size_t(1)) - 1;
            uint32_t highest = 0;
            while (index < map_mesh.start_index + map_mesh.num_indices) {
                lowest = glm::min(lowest, map_indices[index]);
                highest = glm::max(highest, map_indices[index]);
                ++index;
            }

            uint32_t vertex_offset = model.vertex_buffer().size();

            index = map_mesh.start_index;
            while (index < map_mesh.start_index + map_mesh.num_indices) {
                model.index_buffer().push_back(
                    (vertex_offset + map_indices[index]) - lowest
                );
                ++index;
            }

            assert(highest > lowest);

            uint32_t n_vertices = highest - lowest + 1u;
            model.vertex_buffer().resize(vertex_offset + n_vertices);
            std::memcpy(
                model.vertex_buffer().data(),
                &map_vertices[lowest],
                n_vertices * sizeof(model.vertex_buffer()[0])
            );
        }

        std::string room_model_path;
        room_model_path = std::string{"assets/models/map/room"} +
                          std::to_string(room_index) +
                          DOT_PRT3_MODEL_EXT;

        model.save_prt3model(room_model_path.c_str());
        model.set_path(room_model_path);

        prt3::NodeID room_id = room_scene.add_node_to_root("room");
        prt3::ModelHandle handle = room_scene.upload_model(room_model_path);
        room_scene.add_component<prt3::ModelComponent>(room_id, handle);

        prt3::ColliderComponent & col =
            room_scene.add_component<prt3::ColliderComponent>(
                room_id,
                prt3::ColliderType::collider,
                handle
            );

        /* nav mesh */
        room.nav_mesh_id = map.m_navigation_system.generate_nav_mesh(
            room_id,
            room_scene,
            room_scene.physics_system().get_collision_layer(col.tag()),
            0.5f,
            1.0f,
            1.0f,
            1.0f,
            1.0f
        );

        /* save scene */
        std::string room_scene_path = room_to_scene_path(room_index);
        std::ofstream out(room_scene_path, std::ios::binary);
        room_scene.serialize(out);
        out.close();

#ifdef __EMSCRIPTEN__
        prt3::emscripten_save_file_via_put(room_scene_path);
#endif // __EMSCRIPTEN__
    }

    /* rename door destinations */
    for (MapDoor & door : map.m_doors) {
        door.dest = num_to_door.at(door.dest);
    }

    return map;
}

#undef TOK_ROOM
#undef TOK_DOOR

std::string Map::room_to_scene_path(RoomID room_id) {
    return std::string{"assets/scenes/map/room"} +
           std::to_string(room_id) +
           ".prt3";
}

void Map::serialize(std::ofstream & out) {
    prt3::write_stream(out, m_rooms.size());
    for (size_t i = 0; i < m_rooms.size(); ++i) {
        prt3::write_stream(out, m_rooms[i].doors.start_index);
        prt3::write_stream(out, m_rooms[i].doors.num_indices);
        m_navigation_system.serialize_nav_mesh(m_rooms[i].nav_mesh_id, out);
    }

    prt3::write_stream(out, m_doors.size());
    for (size_t i = 0; i < m_doors.size(); ++i) {
        prt3::write_stream(out, m_doors[i].position.position);
        prt3::write_stream(out, m_doors[i].position.room);
        prt3::write_stream(out, m_doors[i].dest);
    }

    prt3::write_stream(out, m_locations.size());
    for (auto const & pair : m_location_ids) {
        prt3::write_stream(out, pair.first.length());
        out.write(pair.first.c_str(), pair.first.length());
        prt3::write_stream(out, m_locations[pair.second].position);
        prt3::write_stream(out, m_locations[pair.second].room);
    }
}

void Map::deserialize(std::ifstream & in) {
    size_t n_rooms;
    prt3::read_stream(in, n_rooms);
    m_rooms.resize(n_rooms);
    for (size_t i = 0; i < m_rooms.size(); ++i) {
        prt3::read_stream(in, m_rooms[i].doors.start_index);
        prt3::read_stream(in, m_rooms[i].doors.num_indices);
        m_rooms[i].nav_mesh_id =
            m_navigation_system.deserialize_nav_mesh(i, in);
    }

    size_t n_doors;
    prt3::read_stream(in, n_doors);
    m_doors.resize(n_doors);
    for (size_t i = 0; i < m_doors.size(); ++i) {
        prt3::read_stream(in, m_doors[i].position.position);
        prt3::read_stream(in, m_doors[i].position.room);
        prt3::read_stream(in, m_doors[i].dest);
    }

    size_t n_locations;
    prt3::read_stream(in, n_locations);
    m_locations.resize(n_locations);
    for (size_t i = 0; i < m_locations.size(); ++i) {
        size_t name_length;
        prt3::read_stream(in, name_length);
        std::string name;
        name.resize(name_length);
        in.read(&name[0], name_length);
        m_location_ids[name] = static_cast<LocationID>(i);

        prt3::read_stream(in, m_locations[i].position);
        prt3::read_stream(in, m_locations[i].room);
    }
}

MapPathID Map::query_map_path(MapPosition origin, MapPosition destination) {
    MapPathID id = m_next_map_path_id;

    MapPath & mp = *m_map_path_cache.push_new_entry(id);
    if (!get_map_path(origin, destination, mp.path)) {
        m_map_path_cache.invalidate(id);
        return NO_MAP_PATH;
    }

    float length = 0.0f;
    for (uint32_t i = 1; i < mp.path.size(); ++i) {
        length += glm::distance(
            mp.path[i].position.position,
            mp.path[i - 1].position.position
        );
    }
    mp.length = length;

    ++m_next_map_path_id;
    return id;
}

MapPosition Map::interpolate_map_path(MapPathID id, float t) {
    MapPath & mp = *m_map_path_cache.access(id);
    float target_dist = mp.length * t;
    uint32_t l = 0;
    uint32_t r = mp.path.size() - 1;

    while (l <= r) {
        uint32_t m = (l + r) / 2;
        if (mp.path[m].accumulated_distance <= target_dist) {
            l = m;
        } else {
            r = m - 1;
        }
    }
    float i = mp.path[l].accumulated_distance <= target_dist ? l : l - 1;
    if (i + 1 == mp.path.size() ||
        mp.path[i].position.room != mp.path[i + 1].position.room) {
        return mp.path.back().position;
    }

    float remaining = target_dist - mp.path[i].accumulated_distance;
    float dist2next = mp.path[i + 1].accumulated_distance -
                      mp.path[i].accumulated_distance;
    float interp = remaining / dist2next;

    MapPosition res;
    res.room = mp.path[i].position.room;
    res.position = glm::mix(
        mp.path[i].position.position,
        mp.path[i + 1].position.position,
        interp
    );

    return res;
}

bool Map::get_map_path(
    MapPosition from,
    MapPosition to,
    std::vector<MapPathEntry> & path
) {
    path.clear();

    struct QElem {
        uint32_t index;
        float total_dist;
    };

    struct Compare {
        bool operator() (QElem const & l, QElem const & r) const
        {
            return l.total_dist > r.total_dist;
        }
    } compare;

    typedef std::priority_queue<QElem, std::vector<QElem>, Compare> CostQueue;
    thread_local CostQueue q{compare};
    while (!q.empty()) { q.pop(); }

    struct GInfo {
        int32_t prev;
        float dist;
    };

    thread_local std::unordered_map<uint32_t, GInfo> info;
    info.clear();

    if (from.room == to.room) {
        float length = get_nav_path_length(
            from.room,
            from.position,
            to.position
        );

        uint32_t vi =  m_doors.size(); // virtual index
        if (length >= 0.0f) {
            info[vi].prev = -2;
            info[vi].dist = length;
            q.push(QElem{vi, length});
        }
    }

    MapRoom const & room_start = m_rooms[from.room];
    for (uint32_t i = room_start.doors.start_index;
         i < room_start.doors.start_index + room_start.doors.num_indices;
         ++i) {
        float length = get_nav_path_length(
            from.room,
            from.position,
            m_doors[i].position.position
        );

        if (length >= 0.0f) {
            info[i].prev = -1;
            info[i].dist = length;
            q.push(QElem{i, length});
        }
    }

    int32_t curr = -1; // for constructing the path

    while (!q.empty()) {
        QElem qe = q.top();
        q.pop();

        if (qe.index >= m_doors.size()) {
            curr = info[curr].prev;
            break;
        }

        MapDoor const & door = m_doors[m_doors[qe.index].dest];
        MapRoom const & room = m_rooms[door.position.room];

        if (door.position.room == to.room) {
            NavPath const * np = get_nav_path(
                door.position.room,
                door.position.position,
                to.position
            );

            if (np) {
                float dist = qe.total_dist + np->length;

                uint32_t di = qe.index - room.doors.start_index;
                uint32_t vi = m_doors.size() + di + 1; // virtual index
                if (info.find(vi) == info.end() ||
                    info.at(vi).dist > dist) {
                    info.at(vi).prev = qe.index;
                    q.push(QElem{vi, dist});
                }
            }
        }

        for (uint32_t i = room.doors.start_index;
            i < room.doors.start_index + room.doors.num_indices;
            ++i) {
            if (m_doors[i].dest == qe.index) continue;
            float length = get_nav_path_length(
                door.position.room,
                door.position.position,
                m_doors[i].position.position
            );

            if (length >= 0.0f) {
                float dist = qe.total_dist + length;

                if (info.find(i) == info.end() ||
                    info.at(i).dist > dist) {
                    info.at(i).prev = qe.index;
                    q.push(QElem{i, dist});
                }
            }
        }
    }

    if (curr == -1) return false;
    /* create path by backtracking and then reverse it */

    if (curr == -2) {
        /* we never leave the room */
        NavPath const * np = get_nav_path(
            from.room,
            to.position,
            from.position
        );

        for (glm::vec3 const & pos : np->path) {
            MapPathEntry path_entry;
            path_entry.position.room = from.room;
            path_entry.position.position = pos;
            path.push_back(path_entry);
        }
    } else {
        /* insert from dest to last door */
        NavPath const * np = get_nav_path(
            to.room,
            to.position,
            m_doors[curr].position.position
        );

        for (glm::vec3 const & pos : np->path) {
            MapPathEntry path_entry;
            path_entry.position.room = m_doors[curr].position.room;
            path_entry.position.position = pos;
            path.push_back(path_entry);
        }
    }

    while (info.at(curr).prev >= 0) {
        NavPath const * np = get_nav_path(
            m_doors[curr].position.room,
            m_doors[curr].position.position,
            m_doors[info.at(curr).prev].position.position
        );

        for (glm::vec3 const & pos : np->path) {
            MapPathEntry path_entry;
            path_entry.position.room = m_doors[curr].position.room;
            path_entry.position.position = pos;
            path.push_back(path_entry);
        }

        curr = info.at(curr).prev;
    }

    {
        NavPath const * np = get_nav_path(
            from.room,
            m_doors[curr].position.position,
            from.position
        );

        for (glm::vec3 const & pos : np->path) {
            MapPathEntry path_entry;
            path_entry.position.room = m_doors[curr].position.room;
            path_entry.position.position = pos;
            path.push_back(path_entry);
        }
    }

    std::reverse(path.begin(), path.end());

    path[0].accumulated_distance = 0.0f;
    for (uint32_t i = 1; i < path.size(); ++i) {
        float dist = 0.0f;
        if (path[i].position.room == path[i-1].position.room) {
            dist = glm::distance(
                path[i].position.position,
                path[i-1].position.position
            );
        }
        path[i].accumulated_distance = path[i-1].accumulated_distance + dist;
    }

    return true;
}

Map::NavPath const * Map::get_nav_path(
    RoomID room_id,
    glm::vec3 from,
    glm::vec3 to
) {
    NavPathKey key;
    key.room = room_id;
    key.from = from;
    key.to = to;

    NavPath * np = m_nav_mesh_path_cache.access(key);
    if (np) {
        return np;
    }

    np = m_nav_mesh_path_cache.push_new_entry(key);

    m_navigation_system.generate_path(
        m_rooms[room_id].nav_mesh_id,
        from,
        to,
        np->path
    );

    if (np->path.empty()) {
        m_nav_mesh_path_cache.invalidate(key);
        return nullptr;
    }

    float length = 0.0f;
    for (uint32_t i = 1; i < np->path.size(); ++i) {
        length += glm::distance(np->path[i], np->path[i-1]);
    }
    np->length = length;

    if (!m_nav_mesh_path_length_cache.has_key(key)) {
        float * l = m_nav_mesh_path_length_cache.push_new_entry(key);
        *l = length;
    }

    return np;
}

float Map::get_nav_path_length(
    RoomID room_id,
    glm::vec3 from,
    glm::vec3 to
) {
    NavPathKey key;
    key.room = room_id;
    key.from = from;
    key.to = to;

    float * l = m_nav_mesh_path_length_cache.access(key);
    if (l) {
        return *l;
    }

    get_nav_path(room_id, from, to);

    l = m_nav_mesh_path_length_cache.access(key);
    if (!l) {
        return -1.0f;
    }
    return *l;
}
