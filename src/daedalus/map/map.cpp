#include "map.h"

#include "src/engine/rendering/model.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/collider_component.h"
#include "src/engine/component/door.h"
#include "src/engine/core/backend_type.h"
#include "src/engine/core/context.h"
#include "src/util/file_util.h"
#include "src/util/log.h"
#include "src/util/sub_vec.h"

#include <cinttypes>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace dds;

#define TOK_ROOM "room"
#define TOK_DOOR "door"

inline bool check_tok(char const * tok, char const * str) {
    return strncmp(tok, str, strlen(tok)) == 0;
}

void dds::parse_map_from_model(char const * path) {
    prt3::Context context{prt3::BackendType::dummy};

    prt3::Model map_model{path};
    auto const & nodes = map_model.nodes();
    auto const & map_vertices = map_model.vertex_buffer();
    auto const & map_indices = map_model.index_buffer();

    std::unordered_map<uint32_t, uint32_t> num_to_room;

    uint32_t index = 0;
    for (prt3::Model::Node const & node : nodes) {
        char const * name = node.name.c_str();

        if (check_tok(TOK_ROOM, name)) {
            uint32_t num;
            sscanf(name + strlen(TOK_ROOM), "%" SCNu32, &num);
            num_to_room[num] = index;
        }
        ++index;
    }

    std::unordered_map<uint32_t, uint32_t> material_map;

    std::vector<prt3::Model> models;
    models.resize(num_to_room.size());

    struct QueueNode {
        prt3::Transform inherited;
        uint32_t index;
        int32_t parent;
    };

    std::vector<QueueNode> node_queue;
    uint32_t room_index = 0;
    for (auto const & pair : num_to_room) {
        prt3::Scene room_scene{context};
        room_scene.ambient_light() = glm::vec3{0.7};

        uint32_t room_num = pair.first;
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

                prt3::NodeID id = room_scene.add_node_to_root(
                (std::string{"door"} + std::to_string(door_id)).c_str()
                );
                room_scene.add_component<prt3::Door>(id);
                prt3::Node & node = room_scene.get_node(id);
                node.set_global_transform(room_scene, global_tform);

                prt3::Door & door_comp = room_scene.get_component<prt3::Door>(id);

                door_comp.id() = door_id;

                std::string dest_scene_path = std::string{"assets/scenes/map/room"} +
                                std::to_string(dest_room) +
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
                          std::to_string(room_num) +
                          DOT_PRT3_MODEL_EXT;

        model.save_prt3model(room_model_path.c_str());
        model.set_path(room_model_path);

        prt3::NodeID room_id = room_scene.add_node_to_root("room");
        prt3::ModelHandle handle = room_scene.upload_model(room_model_path);
        room_scene.add_component<prt3::ModelComponent>(room_id, handle);

        room_scene.add_component<prt3::ColliderComponent>(
            room_id,
            prt3::ColliderType::collider,
            handle
        );

        std::string room_scene_path =
                          std::string{"assets/scenes/map/room"} +
                          std::to_string(room_num) +
                          ".prt3";

        std::ofstream out(room_scene_path, std::ios::binary);
        room_scene.serialize(out);
        out.close();

#ifdef __EMSCRIPTEN__
        prt3::emscripten_save_file_via_put(room_scene_path);
#endif // __EMSCRIPTEN__

        ++room_index;
    }
}

#undef TOK_ROOM
