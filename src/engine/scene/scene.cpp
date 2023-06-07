#include "scene.h"

#include "src/engine/core/context.h"

#include "src/util/serialization_util.h"

#include <algorithm>

using namespace prt3;

Scene::Scene(Context & context)
 : m_context{&context},
   m_camera{context.renderer().window_width(),
            context.renderer().window_height()}
{
    m_nodes.emplace_back(s_root_id);
    m_node_names.emplace_back("root");
    m_node_mod_flags.emplace_back(Node::ModFlags::none);
}

void Scene::start() {
    m_script_container.start(*this);
}

void Scene::update(float delta_time) {
    m_animation_system.update(*this, delta_time);

    auto & armatures =
        m_component_manager.get_all_components<Armature>();

    for (Armature & armature : armatures) {
        armature.update(*this);
    }

    clear_node_mod_flags();
    m_component_manager.update(*this);
    m_script_container.update(*this, delta_time);
}

void Scene::clear_node_mod_flags() {
    for (Node::ModFlags & flags : m_node_mod_flags) {
        flags = Node::ModFlags::none;
    }
}

NodeID Scene::add_node(NodeID parent_id, const char * name, UUID uuid) {
    NodeID id;
    if (m_free_list.empty()) {
        id = m_nodes.size();
        m_nodes.emplace_back(id);
        m_node_names.emplace_back(name);
        m_node_mod_flags.emplace_back(Node::ModFlags::none);
    } else {
        id = m_free_list.back();
        m_free_list.pop_back();
        m_nodes[id] = {id};
        m_node_names[id] = name;
        m_node_mod_flags[id] = Node::ModFlags::none;
    }

    m_nodes[id].m_parent_id = parent_id;
    m_nodes[parent_id].m_children_ids.push_back(id);
    m_node_uuids[id] = uuid;
    m_uuid_to_node[uuid] = id;

    mark_ancestors(id, Node::ModFlags::descendant_added);

    return id;
}

bool Scene::remove_node(NodeID id) {
    if (id == NO_NODE || id == s_root_id) {
        return false;
    }

    {
        Node & node = get_node(id);
        if (node.parent_id() != NO_NODE) {
            Node & parent = get_node(node.parent_id());

            for (auto it = parent.m_children_ids.begin();
                it != parent.m_children_ids.end(); ++it) {
                if (*it == id) {
                    parent.m_children_ids.erase(it);
                    break;
                }
            }
            mark_ancestors(id, Node::ModFlags::descendant_removed);
        }
    }

    static std::vector<NodeID> queue;
    queue.push_back(id);
    static std::vector<NodeID> to_free_list;
    to_free_list.clear();

    while (!queue.empty()) {
        NodeID q_id = queue.back();
        Node & q_node = get_node(q_id);
        queue.pop_back();

        q_node.m_id = NO_NODE;
        q_node.m_parent_id = NO_NODE;
        m_component_manager.remove_all_components(*this, q_id);

        to_free_list.push_back(q_id);

        auto it = q_node.children_ids().end();
        while (it != q_node.children_ids().begin()) {
            --it;
            queue.push_back(*it);
        }
        q_node.m_children_ids.clear();
    }

    auto it = to_free_list.end();
    while (it != to_free_list.begin()) {
        --it;
        m_free_list.push_back(*it);
    }

    return true;
}

std::string Scene::get_node_path(NodeID id) const {
    NodeID curr_id = id;
    static std::string path;
    path = "";
    while (curr_id != NO_NODE) {
        path = "/" + (get_node_name(curr_id).data() + path);
        curr_id = get_node(curr_id).parent_id();
    }

    return path;
}

void Scene::find_relative_path(
    NodeID a_id,
    NodeID b_id,
    std::vector<int32_t> & result
) const {
    NodeID lca = NO_NODE; // lowest common ancestor

    unsigned int depth_a = 0;
    unsigned int depth_b = 0;
    NodeID a_curr_id = a_id;
    while (a_curr_id != get_root_id()) {
        ++depth_a;
        a_curr_id = get_node(a_curr_id).parent_id();
    }
    NodeID b_curr_id = b_id;
    while (b_curr_id != get_root_id()) {
        ++depth_b;
        b_curr_id = get_node(b_curr_id).parent_id();
    }

    a_curr_id = a_id;
    b_curr_id = b_id;
    if (depth_a > depth_b) {
        unsigned int diff = depth_a - depth_b;
        while (diff != 0) {
            a_curr_id = get_node(a_curr_id).parent_id();
            --diff;
        }
    } else if (depth_a < depth_b) {
        unsigned int diff = depth_b - depth_a;
        while (diff != 0) {
            b_curr_id = get_node(b_curr_id).parent_id();
            --diff;
        }
    }

    while (a_curr_id != b_curr_id) {
        a_curr_id = get_node(a_curr_id).parent_id();
        b_curr_id = get_node(b_curr_id).parent_id();
    }
    lca = a_curr_id;

    static std::vector<int32_t> reverse;
    reverse.resize(0);

    NodeID curr_id = b_id;
    while (curr_id != lca) {
        NodeID parent_id = get_node(curr_id).parent_id();

        uint32_t child_ind = 0;
        for (NodeID const & id : get_node(parent_id).children_ids()) {
            if (id == curr_id) {
                break;
            }
            ++child_ind;
        }

        reverse.push_back(child_ind);
        curr_id = parent_id;
    }

    curr_id = a_id;
    while (curr_id != lca) {
        reverse.push_back(-1); // -1 == UP
        curr_id = get_node(curr_id).parent_id();
    }

    result.resize(reverse.size());

    for (unsigned int i = 0; i < result.size(); ++i) {
        result[i] = reverse[reverse.size() - (i + 1)];
    }
}

NodeID Scene::get_node_id_from_relative_path(
    NodeID id,
    std::vector<int32_t> const & path
) {
    NodeID res = id;
    for (int32_t i : path) {
        if (i == -1) {
            res = get_node(res).parent_id();
        } else {
            res = get_node(res).children_ids()[i];
        }
    }
    return res;
}

Input & Scene::get_input() {
    return m_context->input();
}

void Scene::collect_world_render_data(
    WorldRenderData & world_data,
    NodeID selected
) {
    m_transform_cache.collect_global_transforms(
        m_nodes.data(),
        m_nodes.size(),
        s_root_id
    );

    m_physics_system.update(
        m_transform_cache.global_transforms().data(),
        m_transform_cache.global_transforms_history().data()
    );

    auto & armatures =
        m_component_manager.get_all_components<Armature>();

    for (Armature const & armature : armatures) {
        glm::mat4 tform =
            m_transform_cache.global_transforms()[armature.node_id()].to_matrix();

        glm::mat4 inv = glm::inverse(tform);

        Model const & model = model_manager().get_model(armature.model_handle());
        auto const & bones = model.bones();

        Animation & animation =
            m_animation_system.m_animations[armature.animation_id()];

        for (Armature::BonePair const & pair : armature.m_bone_map) {
            if (!node_exists(pair.node_id)) continue;

            animation.transforms[pair.bone_index] =
                bones[pair.bone_index].inverse_mesh_transform *
                inv *
                m_transform_cache.global_transforms()[pair.node_id].to_matrix() *
                bones[pair.bone_index].offset_matrix;
        }
    }

    std::vector<Animation> const & animations =
        m_animation_system.animations();

    world_data.bone_data.resize(animations.size() + 1);
    size_t bone_data_back_index = animations.size();
    for (glm::mat4 & bone : world_data.bone_data[bone_data_back_index].bones) {
        bone = glm::mat4{1.0f};
    }

    size_t bone_data_i = 0;
    for (Animation const & animation : animations) {
        BoneData & bone_data = world_data.bone_data[bone_data_i];

        assert(animation.transforms.size() < bone_data.bones.size());

        for (size_t i = 0; i < animation.transforms.size(); ++i) {
            bone_data.bones[i] = animation.transforms[i];
        }

        ++bone_data_i;
    }

    std::vector<Transform> const & global_transforms =
        m_transform_cache.global_transforms();

    static std::unordered_set<NodeID> selected_incl_children;
    selected_incl_children.clear();
    static std::vector<NodeID> queue;
    if (selected != NO_NODE) {
        queue.push_back(selected);
    }

    while (!queue.empty()) {
        NodeID curr = queue.back();
        Node const & curr_node = get_node(curr);
        queue.pop_back();
        selected_incl_children.insert(curr);

        for (NodeID const & child_id : curr_node.children_ids()) {
            queue.push_back(child_id);
        }
    }

    auto const & mesh_comps = m_component_manager.get_all_components<Mesh>();
    for (auto const & mesh_comp : mesh_comps) {
        if (mesh_comp.resource_id() == NO_RESOURCE) {
            continue;
        }

        NodeID id = mesh_comp.node_id();
        MeshRenderData mesh_data;
        mesh_data.mesh_id = mesh_comp.resource_id();
        mesh_data.node = id;

        mesh_data.transform = global_transforms[id].to_matrix();
        mesh_data.material_id = has_component<MaterialComponent>(id) ?
            get_component<MaterialComponent>(id).resource_id() : NO_RESOURCE;

        Model const & model =
            model_manager().get_model_from_mesh_id(mesh_comp.resource_id());

        if (!model.is_animated()) {
            world_data.mesh_data.push_back(mesh_data);
        } else {
            AnimatedMeshRenderData data;
            data.mesh_data = mesh_data;
            data.bone_data_index = bone_data_back_index;
            world_data.animated_mesh_data.push_back(data);
        }

        if (selected_incl_children.find(id) !=
            selected_incl_children.end()) {
            world_data.selected_mesh_data.push_back(mesh_data);
        }
    }

    auto const & man = model_manager();

    auto const & model_comps = m_component_manager.get_all_components<ModelComponent>();
    for (auto const & model_comp : model_comps) {
        ModelHandle handle = model_comp.model_handle();
        if (handle == NO_MODEL) {
            continue;
        }
        NodeID id = model_comp.node_id();
        auto const & resources = man.model_resources().at(handle);
        auto const & model = man.get_model(handle);

        for (size_t i = 0; i < resources.mesh_resource_ids.size(); ++i) {
            auto const & model_node =
                model.nodes()[model.meshes()[i].node_index];
            MeshRenderData mesh_data;
            mesh_data.mesh_id = resources.mesh_resource_ids[i];
            mesh_data.material_id = resources.material_resource_ids[i];
            mesh_data.node = id;
            mesh_data.transform =
                global_transforms[id].to_matrix()
                * model_node.inherited_transform.to_matrix();

            if (!model.is_animated()) {
                world_data.mesh_data.push_back(mesh_data);
            } else {
                AnimatedMeshRenderData data;
                data.mesh_data = mesh_data;
                data.bone_data_index = bone_data_back_index;
                world_data.animated_mesh_data.push_back(data);
            }

            if (selected_incl_children.find(id) !=
                selected_incl_children.end()) {
                world_data.selected_mesh_data.push_back(mesh_data);
            }
        }
    }

    auto const & anim_model_comps =
        m_component_manager.get_all_components<AnimatedModel>();

    for (auto const & model_comp : anim_model_comps) {
        ModelHandle handle = model_comp.model_handle();
        if (handle == NO_MODEL) {
            continue;
        }
        NodeID id = model_comp.node_id();
        auto const & resources = man.model_resources().at(handle);
        auto const & model = man.get_model(handle);

        AnimationID anim_id = model_comp.animation_id();

        for (size_t i = 0; i < resources.mesh_resource_ids.size(); ++i) {
            auto const & model_node =
                model.nodes()[model.meshes()[i].node_index];

            AnimatedMeshRenderData data;
            MeshRenderData & mesh_data = data.mesh_data;
            mesh_data.mesh_id = resources.mesh_resource_ids[i];
            mesh_data.material_id = resources.material_resource_ids[i];
            mesh_data.node = id;
            mesh_data.transform =
                global_transforms[id].to_matrix()
                * model_node.inherited_transform.to_matrix();

            data.mesh_data = mesh_data;
            data.bone_data_index = anim_id;
            world_data.animated_mesh_data.push_back(data);

            if (selected_incl_children.find(id) !=
                selected_incl_children.end()) {
                world_data.selected_animated_mesh_data.push_back(data);
            }
        }
    }

    auto const & anim_mesh_comps =
        m_component_manager.get_all_components<AnimatedMesh>();
    for (auto const & mesh_comp : anim_mesh_comps) {
        if (mesh_comp.resource_id() == NO_RESOURCE) {
            continue;
        }

        NodeID id = mesh_comp.node_id();
        MeshRenderData mesh_data;
        mesh_data.mesh_id = mesh_comp.resource_id();
        mesh_data.node = id;

        mesh_data.transform = global_transforms[id].to_matrix();
        mesh_data.material_id = has_component<MaterialComponent>(id) ?
            get_component<MaterialComponent>(id).resource_id() : NO_RESOURCE;

        ModelHandle model_handle =
            model_manager().get_model_handle_from_mesh_id(
                mesh_comp.resource_id()
            );

        NodeID armature_id = mesh_comp.armature_id();

        AnimationID anim_id = bone_data_back_index;
        if (armature_id != NO_NODE &&
            node_exists(armature_id) &&
            has_component<Armature>(armature_id) &&
            get_component<Armature>(armature_id).model_handle() == model_handle
        ) {
            Armature const & armature = get_component<Armature>(armature_id);

            anim_id = armature.animation_id();
        }

        AnimatedMeshRenderData data;
        data.mesh_data = mesh_data;
        data.bone_data_index = anim_id;
        world_data.animated_mesh_data.push_back(data);

        if (selected_incl_children.find(id) !=
            selected_incl_children.end()) {
            world_data.selected_animated_mesh_data.push_back(data);
        }
    }

    std::vector<PointLightRenderData> point_lights;
    auto const & lights
        = m_component_manager.get_all_components<PointLightComponent>();
    for (auto const & light : lights) {
        PointLightRenderData point_light_data;
        point_light_data.light = light.light();
        point_light_data.position = global_transforms[light.node_id()].to_matrix()
                                        * glm::vec4(0.0f,0.0f,0.0f,1.0f);

        point_lights.push_back(point_light_data);
    }

    glm::vec3 camera_position = m_camera.get_position();
    std::sort(point_lights.begin(), point_lights.end(),
        [&camera_position](PointLightRenderData const & a, PointLightRenderData const & b) {
            return glm::distance2(a.position, camera_position) <
                   glm::distance2(b.position, camera_position);
        });

    world_data.light_data.number_of_point_lights =
        point_lights.size() < LightRenderData::MAX_NUMBER_OF_POINT_LIGHTS ?
            point_lights.size() : LightRenderData::MAX_NUMBER_OF_POINT_LIGHTS;
    for (size_t i = 0; i < world_data.light_data.number_of_point_lights; ++i) {
        world_data.light_data.point_lights[i] = point_lights[i];
    }

    world_data.light_data.directional_light = m_directional_light;
    world_data.light_data.directional_light_on = m_directional_light_on;

    world_data.light_data.ambient_light = m_ambient_light;
}

void Scene::update_window_size(int w, int h) {
    m_camera.set_size(w, h);
}

void Scene::emit_signal(SignalString const & signal, void * data) {
    for (Script * script : m_signal_connections[signal]) {
        script->on_signal(*this, signal, data);
    }
}

ModelManager const & Scene::model_manager() const {
    return m_context->model_manager();
}

ModelManager & Scene::model_manager() {
    return m_context->model_manager();
}

Model const & Scene::get_model(ModelHandle handle) const {
    return m_context->model_manager().get_model(handle);
}

SceneManager & Scene::scene_manager() {
    return m_context->scene_manager();
}

void Scene::serialize(std::ostream & out) const {
    static std::unordered_map<NodeID, NodeID> compacted_ids;
    compacted_ids.clear();
    compacted_ids[NO_NODE] = NO_NODE;

    NodeID n_compacted = 0;
    for (Node const & node : m_nodes) {
        if (node.id() != NO_NODE) {
            compacted_ids[node.id()] = n_compacted;
            ++n_compacted;
        }
    }

    write_stream(out, n_compacted);

    for (NodeID id = 0; id < n_compacted; ++id) {
        write_stream(out, m_node_uuids.at(id));
    }

    for (Node const & node : m_nodes) {
        if (node.id() == NO_NODE) {
            continue;
        }
        auto const & name = m_node_names[node.id()];
        out.write(name.data(), name.writeable_size());
        out << node.local_transform();
        NodeID parent_id = compacted_ids.at(node.parent_id());
        write_stream(out, parent_id);
        NodeID n_children = node.children_ids().size();
        write_stream(out, n_children);

        for (NodeID const & child_id : node.children_ids()) {
            write_stream(out, compacted_ids.at(child_id));
        }
    }

    out << m_directional_light;
    write_stream(out, m_directional_light_on);
    out << m_ambient_light;

    m_component_manager.serialize(out, *this, compacted_ids);
}

void Scene::deserialize(std::istream & in) {
    internal_clear(false);

    NodeID n_nodes;
    read_stream(in, n_nodes);
    for (NodeID id = 0; id < n_nodes; ++id) {
        UUID uuid;
        read_stream(in, uuid);
        m_node_uuids[id] = uuid;
        m_uuid_to_node[uuid] = id;
    }

    for (NodeID id = 0; id < n_nodes; ++id) {
        m_node_names.push_back({});
        m_node_mod_flags.push_back(Node::ModFlags::none);
        auto & name = m_node_names.back();
        in.read(name.data(), name.writeable_size());

        m_nodes.push_back({id});
        Node & node = m_nodes.back();

        in >> node.local_transform();
        read_stream(in, node.m_parent_id);

        NodeID n_children;
        read_stream(in, n_children);
        node.m_children_ids.resize(n_children);
        for (NodeID & child_id : node.m_children_ids) {
            read_stream(in, child_id);
        }
    }

    in >> m_directional_light;
    read_stream(in, m_directional_light_on);
    in >> m_ambient_light;

    m_component_manager.deserialize(in, *this);
}

void Scene::internal_clear(bool place_root) {
    m_nodes.clear();
    m_node_names.clear();
    m_node_mod_flags.clear();
    if (place_root) {
        m_nodes.emplace_back(s_root_id);
        m_node_names.emplace_back("root");
        m_node_mod_flags.emplace_back(Node::ModFlags::none);
    }

    m_component_manager.clear();
    m_physics_system.clear();
    m_animation_system.clear();
    m_navigation_system.clear();

    m_directional_light = {};
    m_directional_light_on = false;
    m_ambient_light = {{0.5f, 0.5f, 0.05f}};

    m_transform_cache.clear();

    m_script_container.clear();

    m_signal_connections.clear();

    m_tags.clear();
    m_node_to_tag.clear();

    m_camera.transform() = {};

    m_referenced_models.clear();

    m_autoload_scripts.clear();
}

void Scene::add_autoload_scripts(std::unordered_set<UUID> const & uuids) {
    for (UUID uuid : uuids) {
        NodeID node_id = add_node_to_root(Script::get_script_name(uuid));
        add_component<ScriptSet>(node_id);

        ScriptSet & script_set = get_component<ScriptSet>(node_id);
        ScriptID script_id = script_set.add_script_from_uuid(*this, uuid);
        m_autoload_scripts[uuid] = script_id;
    }
}
