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
void inner_show_component<Material>(
    EditorContext & context,
    NodeID id
) {
    Scene & scene = context.scene();
    ModelManager & man = context.get_model_manager();

    Material & component = scene.get_component<Material>(id);
    ResourceID resource_id = component.resource_id();

    ImGui::PushItemWidth(160);

    if (resource_id != NO_RESOURCE) {
        Model const & model = man.get_model_from_material_id(resource_id);
        std::vector<Model::Material> const & materials = model.materials();

        uint32_t mesh_index = man.get_mesh_index_from_material_id(resource_id);
        Model::Mesh const & mesh = model.meshes()[mesh_index];
        uint32_t material_index = mesh.material_index;

        Model::Material const & material = materials[material_index];

        static FixedString<64> name;
        name = material.name.c_str();
        ImGui::InputText(
            "name",
            name.data(),
            name.size(),
            ImGuiInputTextFlags_ReadOnly
        );

        glm::vec4 albedo = material.albedo;
        float* albedo_p = reinterpret_cast<float*>(&albedo);
        ImGui::ColorEdit4("albedo", albedo_p);

        float metallic = material.metallic;
        ImGui::InputFloat(
            "metallic",
            &metallic,
            0.0f,
            0.0f,
            "%.2f",
            ImGuiInputTextFlags_ReadOnly
        );

        float roughness = material.roughness;
        ImGui::InputFloat(
            "roughness",
            &roughness,
            0.0f,
            0.0f,
            "%.2f",
            ImGuiInputTextFlags_ReadOnly
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
        std::vector<Model> const & models = man.models();

        ModelManager::ModelHandle handle = 0;
        for (Model const & model : models) {
            if (ImGui::BeginMenu(model.path().c_str())) {
                size_t mesh_index = 0;
                for (Model::Material const & material : model.materials()) {
                    if (ImGui::MenuItem(material.name.c_str())) {
                        ResourceID material_id =
                            man.get_material_id_from_mesh_index(
                                handle,
                                mesh_index
                            );

                        component.m_resource_id = material_id;
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
