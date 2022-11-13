#ifndef PRT3_MATERIAL_GUI_H
#define PRT3_MATERIAL_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/material.h"
#include "src/util/fixed_string.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<MaterialComponent>(
    EditorContext & context,
    NodeID id
) {
    Scene & scene = context.scene();
    MaterialManager & man = context.get_material_manager();

    MaterialComponent & component = scene.get_component<MaterialComponent>(id);
    ResourceID resource_id = component.resource_id();

    ImGui::PushItemWidth(160);

    if (resource_id != NO_RESOURCE) {
        Material & material = man.get_material(resource_id);

        static FixedString<64> name;
        name = material.name.c_str();
        ImGui::InputText(
            "name",
            name.data(),
            name.size(),
            ImGuiInputTextFlags_ReadOnly
        );

        float* albedo_p = reinterpret_cast<float*>(&material.albedo);
        ImGui::ColorEdit4("albedo", albedo_p);

        ImGui::DragFloat(
            "metallic",
            &material.metallic,
            0.01f,
            0.0f,
            1.0f,
            "%.2f"
        );

        ImGui::DragFloat(
            "roughness",
            &material.roughness,
            0.01f,
            0.0f,
            1.0f,
            "%.2f"
        );

        static FixedString<64> albedo_map;
        albedo_map = material.albedo_map.c_str();
        ImGui::InputText(
            "albedo map",
            albedo_map.data(),
            albedo_map.size(),
            ImGuiInputTextFlags_ReadOnly
        );

        static FixedString<64> normal_map;
        normal_map = material.normal_map.c_str();
        ImGui::InputText(
            "normal map",
            normal_map.data(),
            normal_map.size(),
            ImGuiInputTextFlags_ReadOnly
        );

        static FixedString<64> metallic_map;
        metallic_map = material.metallic_map.c_str();
        ImGui::InputText(
            "metallic map",
            metallic_map.data(),
            metallic_map.size(),
            ImGuiInputTextFlags_ReadOnly
        );

        static FixedString<64> roughness_map;
        roughness_map = material.roughness_map.c_str();
        ImGui::InputText(
            "roughness map",
            roughness_map.data(),
            roughness_map.size(),
            ImGuiInputTextFlags_ReadOnly
        );
    } else {
        ImGui::TextUnformatted("No resource.");
    }

    ImGui::PopItemWidth();

    if (ImGui::Button("Set Material")) {
        ImGui::OpenPopup("select_material_popup");
    }

    ImGui::SameLine();
    if (ImGui::BeginPopup("select_material_popup")) {
        for (ResourceID const & mat_id : man.material_ids()) {
            Material const & material = man.get_material(mat_id);
            if (ImGui::Selectable(material.name.c_str())) {
                component.m_resource_id = mat_id;
            }
        }
        ImGui::EndPopup();
    }
}

} // namespace prt3

#endif
