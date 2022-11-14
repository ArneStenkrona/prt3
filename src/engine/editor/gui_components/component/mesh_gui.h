#ifndef PRT3_MESH_GUI_H
#define PRT3_MESH_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/mesh.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<Mesh>(
    EditorContext & context,
    NodeID id
) {
    Scene & scene = context.scene();
    ModelManager & man = context.get_model_manager();

    Mesh & component = scene.get_component<Mesh>(id);
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
                        ResourceID material_id =
                            man.get_material_id_from_mesh_index(
                                handle,
                                mesh_index
                            );

                        component.m_resource_id = mesh_id;

                        if (!scene.has_component<MaterialComponent>(id)) {
                            scene.add_component<MaterialComponent>(
                                id,
                                material_id
                            );
                        } else {
                            scene.get_component<MaterialComponent>(id)
                                .m_resource_id = material_id;
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
