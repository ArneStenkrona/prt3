#include "map.h"

#include "src/daedalus/objects/interactable.h"
#include "src/daedalus/objects/slide.h"
#include "src/engine/rendering/model.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/collider_component.h"
#include "src/engine/component/door.h"
#include "src/engine/core/backend_type.h"
#include "src/engine/core/context.h"
#include "src/util/file_util.h"
#include "src/util/geometry_util.h"
#include "src/util/log.h"
#include "src/util/serialization_util.h"
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

enum class MapToken {
    none,
    room,
    door,
    location,
    water,
    object,
    interactable,
    slide,
    collider,
    trigger
};

inline bool check_tok(char const * tok, char const * str) {
    return strncmp(tok, str, strlen(tok)) == 0;
}

MapToken get_token(char const * str) {
    if (check_tok(TOK_ROOM, str)) return MapToken::room;
    if (check_tok(TOK_DOOR, str)) return MapToken::door;
    if (check_tok(TOK_LOCATION, str)) return MapToken::location;
    if (check_tok(TOK_WATER, str)) return MapToken::water;
    if (check_tok(TOK_OBJECT, str)) return MapToken::object;
    if (check_tok(TOK_INTERACTABLE, str)) return MapToken::interactable;
    if (check_tok(TOK_SLIDE, str)) return MapToken::slide;
    if (check_tok(TOK_COLLIDER, str)) return MapToken::collider;
    if (check_tok(TOK_TRIGGER, str)) return MapToken::trigger;
    return MapToken::none;
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
    RoomID room_id,
    uint32_t node_index,
    prt3::Transform global_tform,
    char const * name,
    prt3::Scene & scene
) {
    prt3::Model::Node const & model_node =
        ctx.models[room_id].nodes()[node_index];

    int32_t parent_index = model_node.parent_index;

    prt3::NodeID parent_id = scene.root_id();
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
        prt3::Model & obj_model = ctx.object_models[room_id];
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
            ctx.material_maps[room_id]
        );

        ctx.object_meshes[room_id][id] = mesh_index;
    }

    return id;
}

bool Map::parse_door(
    ParsingContext & ctx,
    uint32_t map_node_index,
    RoomID room_id,
    uint32_t node_index,
    prt3::Transform global_tform,
    prt3::Scene & scene
) {
    MapRoom & room = ctx.map.m_rooms[room_id];

    prt3::Model::Node const & model_node =
        ctx.models[room_id].nodes()[node_index];

    uint32_t door_id;
    uint32_t dest_room;
    int32_t dest_door;
    if (sscanf(
        model_node.name.c_str() + strlen(TOK_DOOR),
        "(%" SCNu32 ",%" SCNu32 ",%" SCNi32 ")",
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
    map_door.shape = global_tform;
    map_door.position.room = room_id;
    map_door.position.position = global_tform.position;
    map_door.entry_offset = entry_offset;
    map_door.dest = dest_door; // rename later
    map_door.local_id = door_id;
    ctx.num_to_door[room_id][door_id] = door_ind;
    ctx.door_num_to_dest_room[door_ind] = ctx.map.m_num_to_room_id[dest_room];
    ctx.map.m_doors.emplace_back(map_door);
    ++room.doors.num_indices;

    ctx.map.m_local_ids[std::pair<RoomID, uint32_t>{room_id, door_id}] =
        door_ind;

    prt3::NodeID id = map_node_to_new_scene_node(
        ctx,
        map_node_index,
        room_id,
        node_index,
        global_tform,
        (std::string{"door"} + std::to_string(door_id)).c_str(),
        scene
    );

    if (dest_door == -1) {
        return true;
    }

    prt3::Door & door = scene.add_component<prt3::Door>(id);
    door.id() = door_id;

    RoomID dest_room_id = ctx.map.m_num_to_room_id[dest_room];
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
    RoomID room_id,
    uint32_t node_index,
    prt3::Transform global_tform
) {
    prt3::Model::Node const & model_node =
    ctx.models[room_id].nodes()[node_index];

    if (model_node.name.length() < strlen(TOK_LOCATION)) {
        return false;
    }

    char const * name = model_node.name.c_str() + strlen(TOK_LOCATION);
    MapPosition map_pos;
    map_pos.room = room_id;
    map_pos.position = global_tform.position;
    ctx.map.m_location_ids[name] =
        static_cast<LocationID>(ctx.map.m_locations.size());
    ctx.map.m_locations.push_back(map_pos);

    return true;
}

bool Map::parse_object(
    ParsingContext & ctx,
    uint32_t map_node_index,
    RoomID room_id,
    uint32_t node_index,
    prt3::Transform global_tform,
    prt3::Scene & scene
) {
    map_node_to_new_scene_node(
        ctx,
        map_node_index,
        room_id,
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
    RoomID room_id,
    uint32_t node_index,
    prt3::Transform global_tform,
    prt3::Scene & scene
) {
    prt3::NodeID id = map_node_to_new_scene_node(
        ctx,
        map_node_index,
        room_id,
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
    RoomID room_id,
    uint32_t node_index,
    prt3::Transform global_tform,
    prt3::Scene & scene
) {
    prt3::Model::Node const & model_node =
        ctx.models[room_id].nodes()[node_index];

    prt3::NodeID id = map_node_to_new_scene_node(
        ctx,
        map_node_index,
        room_id,
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
    RoomID room_id,
    uint32_t node_index,
    prt3::Transform global_tform,
    bool is_collider,
    prt3::Scene & scene
) {
    prt3::NodeID id = map_node_to_new_scene_node(
        ctx,
        map_node_index,
        room_id,
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

void Map::generate_nav_mesh(
    prt3::Context & prt3_context,
    ParsingContext & ctx,
    prt3::Model const & model
) {
    std::vector<glm::vec3> triangles;
    for (prt3::Model::Node const & node : model.nodes()) {
        prt3::Transform global_tform = node.inherited_transform;
        glm::mat4 t_mat = global_tform.to_matrix();

        bool include_mesh_in_model = false;
        MapToken token = get_token(node.name.c_str());
        switch (token) {
            case MapToken::none:
            case MapToken::water: /* for now... */ {
                include_mesh_in_model = true;
                break;
            }
            default: {}
        }

        if (!include_mesh_in_model || node.mesh_index == -1) continue;
        prt3::Model::Mesh const & mesh = model.meshes()[node.mesh_index];

        uint32_t end = mesh.start_index + mesh.num_indices;
        size_t curr = triangles.size();
        triangles.resize(curr + mesh.num_indices);

        for (uint32_t i = mesh.start_index; i < end; ++i) {
            glm::vec3 vert =
                model.vertex_buffer()[model.index_buffer()[i]].position;
            vert = t_mat * glm::vec4{vert, 1.0f};
            triangles[curr] = vert;
            ++curr;
        }
    }

    prt3::Scene dummy_scene{prt3_context};
    prt3::NodeID node_id = dummy_scene.add_node_to_root("");

    /* collider */
    prt3::ColliderComponent & col =
        dummy_scene.add_component<prt3::ColliderComponent>(
            node_id,
            prt3::ColliderType::collider,
            std::move(triangles)
        );

    /* nav mesh */
    ctx.map.m_nav_mesh_id = ctx.map.m_navigation_system.generate_nav_mesh(
        node_id,
        dummy_scene,
        dummy_scene.physics_system().get_collision_layer(col.tag()),
        0.5f,
        1.0f,
        1.0f,
        1.0f,
        1.0f
    );
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
            char type_buf[16] = { 0 };
            char name_buf[256] = { 0 };

            int scanned = sscanf(name + strlen(TOK_ROOM), "%" SCNu32 "", &num);

            char const * curr = name + strlen(TOK_ROOM) + scanned;

            /* room type */
            ++curr;
            unsigned i = 0;
            if (*curr) ++curr;
            if (*curr) ++curr;
            while (*curr && *curr != ')') {
                type_buf[i] = *curr;
                ++i;
                ++curr;
            }

            /* room name */
            if (*curr) ++curr;
            if (*curr) ++curr;

            i = 0;
            while (*curr && *curr != ']') {
                name_buf[i] = *curr;
                ++i;
                ++curr;
            }

            if (*name_buf != '\0') {
                ctx.map.m_room_names.push_back(name_buf);
            } else {
                ctx.map.m_room_names.push_back("unkown room");
            }

            ctx.num_to_room_node[num] = index;
            ctx.map.m_num_to_room_id.resize(num + 1);
            ctx.map.m_num_to_room_id[num] = n_rooms;

            MapRoom room{};
            room.type = type_buf[0] == 'i' ? MapRoom::RoomType::indoors :
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
        RoomID room_id = ctx.map.m_num_to_room_id[pair.first];
        MapRoom & room = ctx.map.m_rooms[room_id];
        room.doors.start_index = ctx.map.m_doors.size();

        ctx.model_node_to_scene_node.clear();

        prt3::Scene scene{prt3_context};
        // TODO: Implement a scene name instead since root node name can be
        //       overwritten.
        scene.get_node_name(scene.root_id()) =
            (std::string("room") + std::to_string(room_id)).c_str();

        scene.ambient_light() = glm::vec3{0.7};

        prt3::Model::Node const & room_node = nodes[pair.second];

        material_map.clear();

        prt3::Model & model = ctx.models[room_id];

        prt3::Model::Node new_root{};
        new_root.parent_index = -1;
        new_root.name = room_node.name;
        new_root.transform = room_node.inherited_transform;
        new_root.inherited_transform = room_node.inherited_transform;
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
            MapToken token = get_token(node.name.c_str());
            switch (token) {
                case MapToken::room: {
                    include_mesh_in_model = true;
                    break;
                }
                case MapToken::door: {
                    parse_door(ctx, qni, room_id, node_index, global_tform, scene);
                    break;
                }
                case MapToken::location: {
                    parse_location(ctx, qni, room_id, node_index, global_tform);
                    break;
                }
                case MapToken::water: {
                    /* TODO */
                    include_mesh_in_model = true;
                    break;
                }
                case MapToken::object: {
                    parse_object(ctx, qni, room_id, node_index, global_tform, scene);
                    break;
                }
                case MapToken::slide: {
                    parse_slide(ctx, qni, room_id, node_index, global_tform, scene);
                    break;
                }
                case MapToken::interactable: {
                    parse_interactable(ctx, qni, room_id, node_index, global_tform, scene);
                    break;
                }
                case MapToken::collider: {
                    parse_collider(ctx, qni, room_id, node_index, global_tform, scene);
                    break;
                }
                case MapToken::trigger: {
                    parse_trigger(ctx, qni, room_id, node_index, global_tform, scene);
                    break;
                }
                case MapToken::none: {
                    include_mesh_in_model = true;
                    break;
                }
            }

            if (!include_mesh_in_model || node.mesh_index == -1) continue;
            /* Mesh exists */
            copy_mesh(ctx, node.mesh_index, model, node_index, material_map);
        }

        /* save model */
        std::string room_model_path;
        room_model_path = std::string{"assets/models/map/room"} +
                          std::to_string(room_id) +
                          DOT_PRT3_MODEL_EXT;

        model.save_prt3model(room_model_path.c_str());
        model.set_path(room_model_path);

        prt3::NodeID room_node_id = scene.add_node_to_root("room");
        prt3::ModelHandle handle = scene.upload_model(room_model_path);
        scene.add_component<prt3::ModelComponent>(room_node_id, handle);

        /* collider */
        scene.add_component<prt3::ColliderComponent>(
            room_node_id,
            prt3::ColliderType::collider,
            handle
        );

        /* save object model */
        prt3::Model & obj_model = ctx.object_models[room_id];
        if (!obj_model.meshes().empty()) {
            std::string obj_model_path;
            obj_model_path = std::string{"assets/models/map/objects"} +
                             std::to_string(room_id) +
                             DOT_PRT3_MODEL_EXT;

            obj_model.save_prt3model(obj_model_path.c_str());
            obj_model.set_path(obj_model_path);
            prt3::ModelHandle handle = scene.upload_model(obj_model_path);

            for (auto pair : ctx.object_meshes.at(room_id)) {
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
        std::string room_scene_path = room_to_scene_path(room_id);
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
        RoomID room_id = ctx.door_num_to_dest_room.at(door_ind);
        door.dest = door.dest == -1 ?
            -1 : ctx.num_to_door[room_id].at(door.dest);
    }

    generate_nav_mesh(prt3_context, ctx, map_model);
    ctx.map.init_door_geometry();

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
    char const * root_name = scene.get_node_name(scene.root_id()).data();
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
        prt3::write_stream(out, m_rooms[i].type);
    }

    for (size_t i = 0; i < m_rooms.size(); ++i) {
        prt3::write_stream(out, m_room_names[i].size());
        prt3::write_stream_n(
            out,
            m_room_names[i].data(),
            m_room_names[i].size()
        );
    }

    prt3::write_stream(out, m_num_to_room_id.size());
    for (RoomID id : m_num_to_room_id) {
        prt3::write_stream(out, id);
    }

    prt3::write_stream(out, m_doors.size());
    for (size_t i = 0; i < m_doors.size(); ++i) {
        prt3::write_stream(out, m_doors[i].shape);
        prt3::write_stream(out, m_doors[i].entry_offset);
        prt3::write_stream(out, m_doors[i].dest);
        prt3::write_stream(out, m_doors[i].local_id);
        prt3::write_stream(out, m_doors[i].position.position);
        prt3::write_stream(out, m_doors[i].position.room);
    }

    prt3::write_stream(out, m_locations.size());
    for (auto const & pair : m_location_ids) {
        prt3::write_stream(out, pair.first.length());
        out.write(pair.first.c_str(), pair.first.length());
        prt3::write_stream(out, m_locations[pair.second].position);
        prt3::write_stream(out, m_locations[pair.second].room);
    }

    if (m_nav_mesh_id != prt3::NO_NAV_MESH) {
        prt3::write_stream(out, true);
        m_navigation_system.serialize_nav_mesh(m_nav_mesh_id, out);
    } else {
        prt3::write_stream(out, false);
    }
}

void Map::deserialize(std::ifstream & in) {
    size_t n_rooms;
    prt3::read_stream(in, n_rooms);
    m_rooms.resize(n_rooms);
    for (size_t i = 0; i < m_rooms.size(); ++i) {
        prt3::read_stream(in, m_rooms[i].doors.start_index);
        prt3::read_stream(in, m_rooms[i].doors.num_indices);
        prt3::read_stream(in, m_rooms[i].type);
    }

    m_room_names.resize(n_rooms);
    for (size_t i = 0; i < m_rooms.size(); ++i) {
        size_t size;
        prt3::read_stream(in, size);
        m_room_names[i].resize(size);
        prt3::read_stream_n(
            in,
            m_room_names[i].data(),
            m_room_names[i].size()
        );
    }

    size_t n_num_to_room;
    prt3::read_stream(in, n_num_to_room);
    m_num_to_room_id.resize(n_num_to_room);
    for (RoomID & id : m_num_to_room_id) {
        prt3::read_stream(in, id);
    }

    size_t n_doors;
    prt3::read_stream(in, n_doors);
    m_doors.resize(n_doors);
    for (size_t i = 0; i < m_doors.size(); ++i) {
        prt3::read_stream(in, m_doors[i].shape);
        prt3::read_stream(in, m_doors[i].entry_offset);
        prt3::read_stream(in, m_doors[i].dest);
        prt3::read_stream(in, m_doors[i].local_id);
        prt3::read_stream(in, m_doors[i].position.position);
        prt3::read_stream(in, m_doors[i].position.room);
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

    bool has_nav_mesh;
    prt3::read_stream(in, has_nav_mesh);
    if (has_nav_mesh) {
        m_nav_mesh_id =
            m_navigation_system.deserialize_nav_mesh(0, in);
    }

    for (uint32_t i = 0; i < m_doors.size(); ++i) {
        m_local_ids[std::pair<RoomID, uint32_t>{
            m_doors[i].position.room, m_doors[i].local_id
        }] = i;
    }

    init_door_geometry();
}

MapPathID Map::query_map_path(glm::vec3 origin, glm::vec3 destination) {
    MapPathID id = m_next_map_path_id;

    MapPath & mp = *m_map_path_cache.push_new_entry(id);
    bool path_res = m_navigation_system.generate_path(
        m_nav_mesh_id,
        origin,
        destination,
        mp.path
    );
    if (!path_res) {
        m_map_path_cache.invalidate(id);
        return NO_MAP_PATH;
    }

    mp.curr_ind = 0;

    ++m_next_map_path_id;
    return id;
}

bool Map::advance_map_path(
    MapPathID id,
    glm::vec3 position,
    float delta,
    glm::vec3 & out_pos,
    glm::vec3 & out_dir
) {
    MapPath & mp = *m_map_path_cache.access(id);

    glm::vec3 curr_pos = position;

    float eps = 0.4f;

    float remaining = delta;
    while (mp.curr_ind + 1 < mp.path.size()) {
        /* In practice the nav mesh may hover a bit above ground. Therefore, we
         * need to be more lenient with distance in the y direction.
         */
        float adjust_factor = 0.1f;
        glm::vec3 adjust_y_a = curr_pos;
        glm::vec3 adjust_y_b = mp.path[mp.curr_ind + 1];
        adjust_y_a.y *= adjust_factor;
        adjust_y_b.y *= adjust_factor;

        float dist = glm::distance(
            adjust_y_a,
            adjust_y_b
        );

        if (dist - remaining <= eps) {
            ++mp.curr_ind;
            remaining -= dist;
            curr_pos = mp.path[mp.curr_ind];
        } else {
            break;
        }
    }

    if (mp.curr_ind + 1 >= mp.path.size()) {
        out_pos = mp.path.back();
        return true;
    }

    float seg_dist = glm::distance(
        curr_pos,
        mp.path[mp.curr_ind + 1]
    );

    float t = glm::min(remaining / seg_dist, 1.0f);

    out_pos = glm::mix(
        curr_pos,
        mp.path[mp.curr_ind + 1],
        t
    );

    glm::vec3 dir = mp.path[mp.curr_ind + 1] - curr_pos;
    if (dir != glm::vec3{0.0f}) {
        out_dir = glm::normalize(dir);
    }

    return false;
}

void Map::init_door_geometry() {
    prt3::ColliderTag tag;
    tag.shape = prt3::ColliderShape::mesh;
    tag.type = prt3::ColliderType::collider;

    prt3::CollisionLayer layer = ~0;

    m_door_geometry.resize(m_doors.size() * VERTS_PER_DOOR);

    for (MapRoom & room : m_rooms) {
        uint32_t doors_start = room.doors.start_index;
        uint32_t doors_end = doors_start + room.doors.num_indices;
        for (uint32_t i = doors_start; i < doors_end; ++i) {
            MapDoor const & door = m_doors[i];
            if (door.dest == -1) {
                continue;
            }

            tag.id = i;

            prt3::Box box{};
            box.dimensions = glm::vec3{1.0f};
            box.center.x = box.dimensions.x / 2.0f;
            box.center.y = box.dimensions.y / 2.0f;
            box.center.z = box.dimensions.z / 2.0f;

            prt3::BoxCollider bc{box};

            auto shape = bc.get_shape(door.shape);
            // Order of vertices:
            // 0 : 0, 0, 0
            // 1 : 0, 0, 1
            // 2 : 0, 1, 0
            // 3 : 0, 1, 1
            // 4 : 1, 0, 0
            // 5 : 1, 0, 1
            // 6 : 1, 1, 0
            // 7 : 1, 1, 1
            glm::vec3 v0 = shape.vertices[2];
            glm::vec3 v1 = shape.vertices[3];
            glm::vec3 v2 = shape.vertices[7];
            glm::vec3 v3 = shape.vertices[6];

            uint32_t vi = door_to_vertex_index(i);
            m_door_geometry[vi] = v0;
            m_door_geometry[vi + 1] = v1;
            m_door_geometry[vi + 2] = v2;
            m_door_geometry[vi + 3] = v3;

            prt3::AABB aabb;
            aabb.lower_bound = v0;
            aabb.lower_bound = glm::min(aabb.lower_bound, v1);
            aabb.lower_bound = glm::min(aabb.lower_bound, v2);
            aabb.lower_bound = glm::min(aabb.lower_bound, v3);

            aabb.upper_bound = v0;
            aabb.upper_bound = glm::max(aabb.upper_bound, v1);
            aabb.upper_bound = glm::max(aabb.upper_bound, v2);
            aabb.upper_bound = glm::max(aabb.upper_bound, v3);

            room.aabb_tree.insert(tag, layer, aabb);
        }
    }
}
