#include "map.h"

#include "src/daedalus/objects/interactable.h"
#include "src/daedalus/objects/slide.h"
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
#define TOK_DOOR "[door]"
#define TOK_LOCATION "[location]"
#define TOK_WATER "[water]"
#define TOK_OBJECT "[object]"
#define TOK_INTERACTABLE "[interact]"
#define TOK_SLIDE "[slide]"
#define TOK_COLLIDER "[col]"
#define TOK_TRIGGER "[trigger]"

inline bool check_tok(char const * tok, char const * str) {
    return strncmp(tok, str, strlen(tok)) == 0;
}

static constexpr char const * room_scene_path_base = "assets/scenes/map/room";

uint32_t Map::copy_mesh(
    ParsingContext & ctx,
    uint32_t src_mesh_index,
    prt3::Model & dest_model,
    uint32_t node_index,
    std::unordered_map<uint32_t, uint32_t> & material_map
) {
    prt3::Model::Mesh const & src_mesh = ctx.map_model->meshes()[src_mesh_index];
    auto const & src_vertices = ctx.map_model->vertex_buffer();
    auto const & src_indices = ctx.map_model->index_buffer();

    if (material_map.find(src_mesh.material_index) ==
        material_map.end()) {
        material_map[src_mesh.material_index] = dest_model.materials().size();
        dest_model.materials().push_back(
            ctx.map_model->materials()[src_mesh.material_index]
        );
    }

    uint32_t mesh_index = dest_model.meshes().size();
    dest_model.meshes().push_back({});
    prt3::Model::Mesh & dest_mesh = dest_model.meshes().back();
    dest_mesh.start_index = dest_model.index_buffer().size();
    dest_mesh.num_indices = src_mesh.num_indices;
    dest_mesh.start_bone = -1;
    dest_mesh.num_bones = -1;
    dest_mesh.material_index = material_map.at(src_mesh.material_index);
    dest_mesh.node_index = node_index;
    dest_mesh.name = src_mesh.name;

    uint32_t index = src_mesh.start_index;
    uint32_t lowest = glm::max(src_vertices.size(), size_t(1)) - 1;
    uint32_t highest = 0;
    while (index < src_mesh.start_index + src_mesh.num_indices) {
        lowest = glm::min(lowest, src_indices[index]);
        highest = glm::max(highest, src_indices[index]);
        ++index;
    }

    uint32_t vertex_offset = dest_model.vertex_buffer().size();

    index = src_mesh.start_index;
    while (index < src_mesh.start_index + src_mesh.num_indices) {
        dest_model.index_buffer().push_back(
            (vertex_offset + src_indices[index]) - lowest
        );
        ++index;
    }

    assert(highest > lowest);

    uint32_t n_vertices = (highest - lowest) + 1u;
    dest_model.vertex_buffer().resize(vertex_offset + n_vertices);
    std::memcpy(
        &dest_model.vertex_buffer()[vertex_offset],
        &src_vertices[lowest],
        n_vertices * sizeof(dest_model.vertex_buffer()[0])
    );

    dest_model.nodes()[node_index].mesh_index = mesh_index;

    return mesh_index;
}

prt3::NodeID Map::map_node_to_new_scene_node(
    ParsingContext & ctx,
    uint32_t map_node_index,
    uint32_t room_index,
    uint32_t node_index,
    prt3::Transform global_tform,
    char const * name,
    prt3::Scene & scene
) {
    prt3::Model::Node const & model_node =
        ctx.models[room_index].nodes()[node_index];

    int32_t parent_index = model_node.parent_index;

    prt3::NodeID parent_id = scene.get_root_id();
    auto it = ctx.model_node_to_scene_node.find(parent_index);
    if (it != ctx.model_node_to_scene_node.end()) {
        parent_id = it->second;
    }

    prt3::NodeID id = scene.add_node(parent_id, name);
    prt3::Node & node = scene.get_node(id);
    node.set_global_transform(scene, global_tform);

    ctx.model_node_to_scene_node[node_index] = id;

    int32_t map_mesh_index = ctx.map_model->nodes()[map_node_index].mesh_index;
    if (map_mesh_index != -1) {
        prt3::Model & obj_model = ctx.object_models[room_index];
        uint32_t node_index = obj_model.nodes().size();
        obj_model.nodes().push_back({});
        obj_model.nodes()[0].child_indices.push_back(node_index);

        prt3::Model::Node & new_node = obj_model.nodes().back();
        new_node.parent_index = 0;
        new_node.name = model_node.name;

        uint32_t mesh_index = copy_mesh(
            ctx,
            map_mesh_index,
            obj_model,
            node_index,
            ctx.material_maps[room_index]
        );

        ctx.object_meshes[room_index][id] = mesh_index;
    }

    return id;
}

bool Map::parse_door(
    ParsingContext & ctx,
    uint32_t map_node_index,
    uint32_t room_index,
    uint32_t node_index,
    prt3::Transform global_tform,
    prt3::Scene & scene
) {
    MapRoom & room = ctx.map.m_rooms[room_index];

    prt3::Model::Node const & model_node =
        ctx.models[room_index].nodes()[node_index];

    uint32_t door_id;
    uint32_t dest_room;
    uint32_t dest_door;
    if (sscanf(
        model_node.name.c_str() + strlen(TOK_DOOR),
        "(%" SCNu32 ",%" SCNu32 ",%" SCNu32 ")",
        &door_id,
        &dest_room,
        &dest_door
    ) == EOF) {
        return false;
    }

    glm::vec3 offset_dir = -global_tform.get_up();

    glm::vec3 dim_offset =
        -global_tform.get_right() * model_node.transform.scale.x / 2.0f;
    glm::vec3 entry_offset = offset_dir * (model_node.transform.scale.y) + dim_offset;

    uint32_t door_ind = ctx.map.m_doors.size();
    MapDoor map_door;
    map_door.position.room = room_index;
    map_door.position.position = global_tform.position + entry_offset;
    map_door.dest = dest_door; // rename later
    ctx.num_to_door[room_index][door_id] = door_ind;
    ctx.door_num_to_dest_room[door_ind] = ctx.num_to_room_index.at(dest_room);
    ctx.map.m_doors.emplace_back(map_door);
    ++room.doors.num_indices;


    prt3::NodeID id = map_node_to_new_scene_node(
        ctx,
        map_node_index,
        room_index,
        node_index,
        global_tform,
        (std::string{"door"} + std::to_string(door_id)).c_str(),
        scene
    );

    prt3::Door & door = scene.add_component<prt3::Door>(id);
    door.id() = door_id;

    RoomID dest_room_id = ctx.num_to_room_index.at(dest_room);
    std::string dest_scene_path = room_to_scene_path(dest_room_id);

    door.destination_scene_path() = dest_scene_path.c_str();
    door.destination_id() = dest_door;

    door.entry_offset() = entry_offset;

    prt3::Box box{};
    box.dimensions = glm::vec3{1.0f};
    box.center.x = box.dimensions.x / 2.0f;
    box.center.y = box.dimensions.y / 2.0f;
    box.center.z = box.dimensions.z / 2.0f;

    scene.add_component<prt3::ColliderComponent>(
        id,
        prt3::ColliderType::area,
        box
    );

    prt3::CollisionLayer layer = 0;
    prt3::CollisionLayer mask = 1 << 15;
    prt3::ColliderComponent & col =
        scene.get_component<prt3::ColliderComponent>(id);
    col.set_layer(scene, layer);
    col.set_mask(scene, mask);

    return true;
}

bool Map::parse_location(
    ParsingContext & ctx,
    uint32_t /*map_node_index*/,
    uint32_t room_index,
    uint32_t node_index,
    prt3::Transform global_tform
) {
    prt3::Model::Node const & model_node =
    ctx.models[room_index].nodes()[node_index];

    if (model_node.name.length() < strlen(TOK_LOCATION)) {
        return false;
    }

    char const * name = model_node.name.c_str() + strlen(TOK_LOCATION);
    MapPosition map_pos;
    map_pos.room = room_index;
    map_pos.position = global_tform.position;
    ctx.map.m_location_ids[name] =
        static_cast<LocationID>(ctx.map.m_locations.size());
    ctx.map.m_locations.push_back(map_pos);

    return true;
}

bool Map::parse_object(
    ParsingContext & ctx,
    uint32_t map_node_index,
    uint32_t room_index,
    uint32_t node_index,
    prt3::Transform global_tform,
    prt3::Scene & scene
) {
    map_node_to_new_scene_node(
        ctx,
        map_node_index,
        room_index,
        node_index,
        global_tform,
        "object",
        scene
    );

    return true;
}

bool Map::parse_interactable(
    ParsingContext & ctx,
    uint32_t map_node_index,
    uint32_t room_index,
    uint32_t node_index,
    prt3::Transform global_tform,
    prt3::Scene & scene
) {
    prt3::NodeID id = map_node_to_new_scene_node(
        ctx,
        map_node_index,
        room_index,
        node_index,
        global_tform,
        "interactable",
        scene
    );

    prt3::ScriptSet & script_set = scene.add_component<prt3::ScriptSet>(id);
    script_set.add_script<Interactable>(scene);

    return true;
}

bool Map::parse_slide(
    ParsingContext & ctx,
    uint32_t map_node_index,
    uint32_t room_index,
    uint32_t node_index,
    prt3::Transform global_tform,
    prt3::Scene & scene
) {
    prt3::Model::Node const & model_node =
        ctx.models[room_index].nodes()[node_index];

    prt3::NodeID id = map_node_to_new_scene_node(
        ctx,
        map_node_index,
        room_index,
        node_index,
        global_tform,
        "slide",
        scene
    );

    glm::vec3 local_displacement;
    if (sscanf(
        model_node.name.c_str() + strlen(TOK_SLIDE),
        "(%f,%f,%f)",
        &local_displacement.x,
        &local_displacement.z,
        &local_displacement.y
    ) == EOF) {
        return false;
    }

    prt3::ScriptSet & script_set = scene.add_component<prt3::ScriptSet>(id);
    prt3::ScriptID script_id = script_set.add_script<Slide>(scene);

    Slide & script = *dynamic_cast<Slide*>(scene.get_script(script_id));
    script.local_displacement() = local_displacement;

    return true;
}

bool Map::parse_collider_trigger_common(
    ParsingContext & ctx,
    uint32_t map_node_index,
    uint32_t room_index,
    uint32_t node_index,
    prt3::Transform global_tform,
    bool is_collider,
    prt3::Scene & scene
) {
    prt3::NodeID id = map_node_to_new_scene_node(
        ctx,
        map_node_index,
        room_index,
        node_index,
        global_tform,
        is_collider ? "collider" : "trigger",
        scene
    );

    /* TODO: parse collider type from node name */
    prt3::Box box{};
    box.dimensions = glm::vec3{2.0f};
    box.center = glm::vec3{0.0f};

    prt3::ColliderType type = is_collider ? prt3::ColliderType::collider :
                                            prt3::ColliderType::area;

    scene.add_component<prt3::ColliderComponent>(
        id,
        type,
        box
    );

    prt3::CollisionLayer layer = 1;
    prt3::CollisionLayer mask = 1 << 15;
    prt3::ColliderComponent & col =
        scene.get_component<prt3::ColliderComponent>(id);
    col.set_layer(scene, layer);
    col.set_mask(scene, mask);

    return true;
}

Map Map::parse_map_from_model(char const * path) {
    ParsingContext ctx;

    prt3::Context prt3_context{prt3::BackendType::dummy};

    prt3::Model map_model{path};
    ctx.map_model = &map_model;

    auto const & nodes = ctx.map_model->nodes();

    uint32_t index = 0;
    uint32_t n_rooms = 0;
    for (prt3::Model::Node const & node : nodes) {
        char const * name = node.name.c_str();

        if (check_tok(TOK_ROOM, name)) {
            uint32_t num;
            char type;
            sscanf(name + strlen(TOK_ROOM), "%" SCNu32 "(%c", &num, &type);
            ctx.num_to_room_node[num] = index;
            ctx.num_to_room_index[num] = n_rooms;

            MapRoom room{};
            room.type = type == 'i' ? MapRoom::RoomType::indoors :
                                      MapRoom::RoomType::outdoors;
            ctx.map.m_rooms.push_back(room);

            ++n_rooms;
        }
        ++index;
    }

    std::unordered_map<uint32_t, uint32_t> material_map;

    ctx.models.resize(ctx.num_to_room_node.size());
    ctx.object_models.resize(ctx.num_to_room_node.size());
    ctx.num_to_door.resize(ctx.num_to_room_node.size());

    for (prt3::Model & obj_model : ctx.object_models) {
        obj_model.nodes().push_back({});
    }

    struct QueueNode {
        prt3::Transform inherited;
        uint32_t index;
        int32_t parent;
    };

    std::vector<QueueNode> node_queue;
    for (auto const & pair : ctx.num_to_room_node) {
        uint32_t room_index = ctx.num_to_room_index.at(pair.first);
        MapRoom & room = ctx.map.m_rooms[room_index];
        room.doors.start_index = ctx.map.m_doors.size();

        prt3::Scene scene{prt3_context};
        // TODO: Implement a scene name instead since root node name can be
        //       overwritten.
        scene.get_node_name(scene.get_root_id()) =
            (std::string("room") + std::to_string(room_index)).c_str();

        scene.ambient_light() = glm::vec3{0.7};

        prt3::Model::Node const & room_node = nodes[pair.second];

        material_map.clear();

        prt3::Model & model = ctx.models[room_index];

        prt3::Model::Node new_root{};
        new_root.parent_index = -1;
        new_root.name = room_node.name;
        new_root.transform = room_node.inherited_transform;
        // new_root.transform.position = glm::vec3{0.0f}; // zero out position
        new_root.inherited_transform = room_node.inherited_transform;
        // new_root.inherited_transform.position = glm::vec3{0.0f}; // zero out position
        model.nodes().push_back(new_root);

        for (auto index : room_node.child_indices) {
            node_queue.push_back({
                prt3::Transform::compose(
                    new_root.inherited_transform,
                    nodes[index].transform
                ),
                index,
                0
            });
        }

        while (!node_queue.empty()) {
            QueueNode qn = node_queue.back();
            uint32_t qni = qn.index;
            node_queue.pop_back();
            prt3::Model::Node const & node = nodes[qni];

            uint32_t node_index = model.nodes().size();

            model.nodes().push_back({});
            prt3::Model::Node & room_node = model.nodes().back();
            room_node.parent_index = qn.parent;
            room_node.transform = node.transform;
            room_node.inherited_transform = node.inherited_transform;
            room_node.name = node.name;

            if (qn.parent != -1) {
                auto & child_indices = model.nodes()[qn.parent].child_indices;
                child_indices.push_back(node_index);
            }

            prt3::Transform global_tform = node.inherited_transform;

            for (auto child_index : node.child_indices) {
                prt3::Transform inherited = prt3::Transform::compose(
                    qn.inherited,
                    nodes[child_index].transform
                );

                node_queue.push_back({
                    inherited,
                    child_index,
                    static_cast<int32_t>(node_index)
                });
            }

            bool include_mesh_in_model = false;
            if (check_tok(TOK_DOOR, node.name.c_str())) {
                parse_door(ctx, qni, room_index, node_index, global_tform, scene);
            } else if (check_tok(TOK_LOCATION, node.name.c_str())) {
                parse_location(ctx, qni, room_index, node_index, global_tform);
            } else if (check_tok(TOK_WATER, node.name.c_str())) {
                /* TODO */
                include_mesh_in_model = true;
            } else if (check_tok(TOK_OBJECT, node.name.c_str())) {
                parse_object(ctx, qni, room_index, node_index, global_tform, scene);
            } else if (check_tok(TOK_SLIDE, node.name.c_str())) {
                parse_slide(ctx, qni, room_index, node_index, global_tform, scene);
            } else if (check_tok(TOK_INTERACTABLE, node.name.c_str())) {
                parse_interactable(ctx, qni, room_index, node_index, global_tform, scene);
            } else if (check_tok(TOK_COLLIDER, node.name.c_str())) {
                parse_collider(ctx, qni, room_index, node_index, global_tform, scene);
            } else if (check_tok(TOK_TRIGGER, node.name.c_str())) {
                parse_trigger(ctx, qni, room_index, node_index, global_tform, scene);
            } else {
                include_mesh_in_model = true;
            }

            if (!include_mesh_in_model || node.mesh_index == -1) continue;
            /* Mesh exists */
            copy_mesh(ctx, node.mesh_index, model, node_index, material_map);
        }

        /* save model */
        std::string room_model_path;
        room_model_path = std::string{"assets/models/map/room"} +
                          std::to_string(room_index) +
                          DOT_PRT3_MODEL_EXT;

        model.save_prt3model(room_model_path.c_str());
        model.set_path(room_model_path);

        prt3::NodeID room_id = scene.add_node_to_root("room");
        prt3::ModelHandle handle = scene.upload_model(room_model_path);
        scene.add_component<prt3::ModelComponent>(room_id, handle);

        /* collider */
        prt3::ColliderComponent & col =
            scene.add_component<prt3::ColliderComponent>(
                room_id,
                prt3::ColliderType::collider,
                handle
            );

        /* nav mesh */
        room.nav_mesh_id = ctx.map.m_navigation_system.generate_nav_mesh(
            room_id,
            scene,
            scene.physics_system().get_collision_layer(col.tag()),
            0.5f,
            1.0f,
            1.0f,
            1.0f,
            1.0f
        );

        /* save object model */
        prt3::Model & obj_model = ctx.object_models[room_index];
        if (!obj_model.meshes().empty()) {
            std::string obj_model_path;
            obj_model_path = std::string{"assets/models/map/objects"} +
                             std::to_string(room_index) +
                             DOT_PRT3_MODEL_EXT;

            obj_model.save_prt3model(obj_model_path.c_str());
            obj_model.set_path(obj_model_path);
            prt3::ModelHandle handle = scene.upload_model(obj_model_path);

            for (auto pair : ctx.object_meshes.at(room_index)) {
                prt3::NodeID node_id = pair.first;
                uint32_t mesh_ind = pair.second;

                prt3::ModelResource const & res =
                    prt3_context.model_manager().get_model_resource(handle);
                prt3::ResourceID mesh_id = res.mesh_resource_ids[mesh_ind];
                prt3::ResourceID mat_id = res.mesh_material_ids[mesh_ind];

                scene.add_component<prt3::Mesh>(node_id, mesh_id);
                scene.add_component<prt3::MaterialComponent>(node_id, mat_id);
            }
        }

        /* save scene */
        std::string room_scene_path = room_to_scene_path(room_index);
        std::ofstream out(room_scene_path, std::ios::binary);
        scene.serialize(out);
        out.close();

#ifdef __EMSCRIPTEN__
        prt3::emscripten_save_file_via_put(room_scene_path);
#endif // __EMSCRIPTEN__
    }

    /* rename door destinations */
    for (uint32_t door_ind = 0; door_ind < ctx.map.m_doors.size(); ++door_ind) {
        MapDoor & door = ctx.map.m_doors[door_ind];
        uint32_t room_index = ctx.door_num_to_dest_room.at(door_ind);
        door.dest = ctx.num_to_door[room_index].at(door.dest);
    }

    return ctx.map;
}

#undef TOK_ROOM
#undef TOK_DOOR

std::string Map::room_to_scene_path(RoomID room_id) {
    return std::string{room_scene_path_base} +
           std::to_string(room_id) +
           ".prt3";
}

RoomID Map::scene_to_room(prt3::Scene const & scene) {
    char const * root_name = scene.get_node_name(scene.get_root_id()).data();
    uint32_t id32;
    int res = sscanf(root_name + strlen("room"), "%" SCNu32, &id32);
    if (res == EOF) {
        return 0;
    }

    RoomID id = id32;
    return id;
}

void Map::serialize(std::ofstream & out) {
    prt3::write_stream(out, m_rooms.size());
    for (size_t i = 0; i < m_rooms.size(); ++i) {
        prt3::write_stream(out, m_rooms[i].doors.start_index);
        prt3::write_stream(out, m_rooms[i].doors.num_indices);

        if (m_rooms[i].nav_mesh_id != prt3::NO_NAV_MESH) {
            prt3::write_stream(out, true);
            m_navigation_system.serialize_nav_mesh(m_rooms[i].nav_mesh_id, out);
        } else {
            prt3::write_stream(out, false);
        }
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

        bool has_nav_mesh;
        prt3::read_stream(in, has_nav_mesh);
        if (has_nav_mesh) {
            m_rooms[i].nav_mesh_id =
                m_navigation_system.deserialize_nav_mesh(i, in);
        }
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

    if (t >= 1.0f) {
        return mp.path.back().position;
    }

    float target_dist = mp.length * t;
    uint32_t l = 0;
    uint32_t r = mp.path.size();

    while (l < r) {
        uint32_t m = (l + r) / 2;
        if (mp.path[m].accumulated_distance <= target_dist) {
            l = m + 1;
        } else {
            r = m;
        }
    }

    uint32_t i = l == 0 ? l : l - 1;

    if (i + 1 == mp.path.size()) {
        return mp.path.back().position;
    }

    if (mp.path[i].position.room != mp.path[i + 1].position.room) {
        return mp.path[i + 1].position;
    }

    float remaining = target_dist - mp.path[i].accumulated_distance;
    float dist2next = mp.path[i + 1].accumulated_distance -
                      mp.path[i].accumulated_distance;
    float interp = dist2next == 0.0f ? 1.0f : remaining / dist2next;

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

        uint32_t vi = m_doors.size(); // virtual index
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
            curr = info.at(qe.index).prev;
            break;
        }

        uint32_t dest_ind = m_doors[qe.index].dest;
        MapDoor const & door = m_doors[dest_ind];
        MapRoom const & room = m_rooms[door.position.room];

        if (door.position.room == to.room) {
            NavPath const * np = get_nav_path(
                door.position.room,
                door.position.position,
                to.position
            );

            if (np) {
                float dist = qe.total_dist + np->length;

                uint32_t vi = m_doors.size() + 1 + dest_ind; // virtual index
                if (info.find(vi) == info.end() ||
                    info.at(vi).dist > dist) {
                    info[vi].prev = qe.index;
                    info[vi].dist = dist;
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
                    info[i].prev = qe.index;
                    info[i].dist = dist;
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
            m_doors[m_doors[curr].dest].position.position
        );

        for (glm::vec3 const & pos : np->path) {
            MapPathEntry path_entry;
            path_entry.position.room = to.room;
            path_entry.position.position = pos;
            path.push_back(path_entry);
        }

        while (info.at(curr).prev >= 0) {
            NavPath const * np = get_nav_path(
                m_doors[curr].position.room,
                m_doors[curr].position.position,
                m_doors[m_doors[info.at(curr).prev].dest].position.position
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
                path_entry.position.room = from.room;
                path_entry.position.position = pos;
                path.push_back(path_entry);
            }
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
