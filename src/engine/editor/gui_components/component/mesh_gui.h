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
            model_path.size(),
            ImGuiInputTextFlags_ReadOnly
        );

        static FixedString<64> name;
        name = mesh.name.c_str();
        ImGui::InputText(
            "name",
            name.data(),
            name.size(),
            ImGuiInputTextFlags_ReadOnly
        );
    } else {
        ImGui::TextUnformatted("No resource.");
    }

    ImGui::PopItemWidth();
}

} // namespace prt3

#endif
