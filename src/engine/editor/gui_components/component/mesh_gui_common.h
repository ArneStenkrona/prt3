#ifndef PRT3_MESH_GUI_COMMON_H
#define PRT3_MESH_GUI_COMMON_H

#include "src/engine/editor/editor_context.h"
#include "src/engine/editor/action/action_set_resource.h"
#include "src/engine/scene/scene.h"

namespace prt3 {

template<typename MeshComponentType>
class ActionSetMesh : public Action {
public:
    ActionSetMesh(
        EditorContext & editor_context,
        NodeID node_id,
        ResourceID mesh_id
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_mesh_id{mesh_id}
    {
        Scene const & scene = editor_context.scene();
        m_original_mesh_id =
            scene.get_component<MeshComponentType>(m_node_id).resource_id();

        m_had_material = scene.has_component<MaterialComponent>(m_node_id);
        if (!m_had_material) {
            m_original_material_id = NO_RESOURCE;
        } else {
            m_original_material_id =
                scene.get_component<MaterialComponent>(m_node_id)
                .resource_id();
        }

        if (m_mesh_id == NO_RESOURCE) {
            m_material_id = m_original_material_id;
        } else {
            ModelManager & man = m_editor_context->get_model_manager();
            auto mesh_index = man.get_mesh_index_from_mesh_id(mesh_id);

            ModelHandle handle = man.get_model_handle_from_mesh_id(mesh_id);

            m_material_id =
                man.get_material_id_from_mesh_index(
                    handle,
                    mesh_index
                );
        }
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();

        scene.get_component<MeshComponentType>(m_node_id).set_resource_id(m_mesh_id);

        if (m_had_material) {
            scene.get_component<MaterialComponent>(m_node_id)
                .set_resource_id(m_material_id);
        } else {
            scene.add_component<MaterialComponent>(m_node_id, m_material_id);
        }

        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<MeshComponentType>(m_node_id).set_resource_id(m_original_mesh_id);

        if (m_had_material) {
            scene.get_component<MaterialComponent>(m_node_id)
                .set_resource_id(m_original_material_id);
        } else {
            scene.remove_component<MaterialComponent>(m_node_id);
        }

        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    ResourceID m_mesh_id;
    ResourceID m_material_id;
    ResourceID m_original_mesh_id;
    ResourceID m_original_material_id;

    bool m_had_material;
};

template<typename MeshComponentType>
void show_mesh_component(
    EditorContext & context,
    NodeID id
) {
    Scene & scene = context.scene();
    ModelManager & man = context.get_model_manager();

    MeshComponentType & component = scene.get_component<MeshComponentType>(id);
    ResourceID resource_id = component.resource_id();

    ImGui::PushItemWidth(160);

    if (resource_id != NO_RESOURCE) {
        Model const & model = man.get_model_from_mesh_id(resource_id);

        uint32_t mesh_index = man.get_mesh_index_from_mesh_id(resource_id);
        Model::Mesh const & mesh = model.meshes()[mesh_index];


        static FixedString<64> model_path;
        model_path = model.path().c_str();
        ImGui::InputText(
            "model",
            model_path.data(),
            model_path.buf_size(),
            ImGuiInputTextFlags_ReadOnly
        );

        static FixedString<64> name;
        name = mesh.name.c_str();
        ImGui::InputText(
            "name",
            name.data(),
            name.buf_size(),
            ImGuiInputTextFlags_ReadOnly
        );
    } else {
        ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "%s", "no mesh");
    }

    ImGui::PopItemWidth();

    if (ImGui::Button("set mesh")) {
        ImGui::OpenPopup("select_mesh_popup");
    }

    ImGui::SameLine();
    if (ImGui::BeginPopup("select_mesh_popup")) {
        std::vector<Model> const & models = man.models();

        ModelHandle handle = 0;
        for (Model const & model : models) {
            if (ImGui::BeginMenu(model.path().c_str())) {
                size_t mesh_index = 0;
                for (Model::Mesh const & mesh : model.meshes()) {
                    if (ImGui::MenuItem(mesh.name.c_str())) {
                        ResourceID mesh_id =
                            man.get_mesh_id_from_mesh_index(
                                handle,
                                mesh_index
                            );
                        if (mesh_id != resource_id) {
                            context.editor()
                            .perform_action<ActionSetMesh<MeshComponentType> >(
                                id,
                                mesh_id
                            );
                        }
                    }
                    ++mesh_index;
                }
                ImGui::EndMenu();
            }
            ++handle;
        }
        ImGui::EndPopup();
    }
}

} // namespace prt3

#endif
