#ifndef PRT3_MATERIAL_GUI_H
#define PRT3_MATERIAL_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/editor/action/action_set_resource.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/material.h"
#include "src/util/fixed_string.h"

#include "imgui.h"

namespace prt3 {

class ActionSetMaterial : public Action {
public:
    ActionSetMaterial(
        EditorContext & editor_context,
        NodeID node_id,
        Material const & material
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_material{material}
    {
        Scene const & scene = editor_context.scene();
        m_resource_id =
            scene.get_component<MaterialComponent>(m_node_id).resource_id();
        MaterialManager const & man = editor_context.get_material_manager();
        m_original_material = man.get_material(m_resource_id);
    }

protected:
    virtual bool apply() {
        MaterialManager & man = m_editor_context->get_material_manager();
        man.get_material(m_resource_id) = m_material;
        return true;
    }

    virtual bool unapply() {
        MaterialManager & man = m_editor_context->get_material_manager();
        man.get_material(m_resource_id) = m_original_material;
        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    ResourceID m_resource_id;
    Material m_material;
    Material m_original_material;
};

template<>
void inner_show_component<MaterialComponent>(
    EditorContext & context,
    NodeID id
) {
    // TODO: Handle materials that are linked to models and materials that are
    //       not.

    Scene & scene = context.scene();
    MaterialManager & man = context.get_material_manager();

    MaterialComponent & component = scene.get_component<MaterialComponent>(id);
    ResourceID resource_id = component.resource_id();

    ImGui::PushItemWidth(160);

    if (resource_id != NO_RESOURCE) {
        Material material = man.get_material(resource_id);
        bool changed = false;

        static FixedString<64> name;
        name = material.name.c_str();
        changed |= ImGui::InputText(
            "name",
            name.data(),
            name.buf_size(),
            ImGuiInputTextFlags_ReadOnly
        );

        float* albedo_p = reinterpret_cast<float*>(&material.albedo);
        changed |= ImGui::ColorEdit4("albedo", albedo_p);

        changed |= ImGui::DragFloat(
            "metallic",
            &material.metallic,
            0.01f,
            0.0f,
            1.0f,
            "%.2f"
        );

        changed |= ImGui::DragFloat(
            "roughness",
            &material.roughness,
            0.01f,
            0.0f,
            1.0f,
            "%.2f"
        );

        static FixedString<64> albedo_map;
        albedo_map = material.albedo_map.c_str();
        changed |= ImGui::InputText(
            "albedo map",
            albedo_map.data(),
            albedo_map.buf_size(),
            ImGuiInputTextFlags_ReadOnly
        );

        static FixedString<64> normal_map;
        normal_map = material.normal_map.c_str();
        changed |= ImGui::InputText(
            "normal map",
            normal_map.data(),
            normal_map.buf_size(),
            ImGuiInputTextFlags_ReadOnly
        );

        static FixedString<64> metallic_map;
        metallic_map = material.metallic_map.c_str();
        changed |= ImGui::InputText(
            "metallic map",
            metallic_map.data(),
            metallic_map.buf_size(),
            ImGuiInputTextFlags_ReadOnly
        );

        static FixedString<64> roughness_map;
        roughness_map = material.roughness_map.c_str();
        changed |= ImGui::InputText(
            "roughness map",
            roughness_map.data(),
            roughness_map.buf_size(),
            ImGuiInputTextFlags_ReadOnly
        );

        changed |= ImGui::Checkbox("transparent", &material.transparent);
        // Two sided rendering is not implemented yet
        // changed |= ImGui::Checkbox("two sided", &material.twosided);

        if (changed) {
            context.editor().perform_action<ActionSetMaterial>(
                id,
                material
            );
        }

    } else {
        ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "%s", "no material");
    }

    ImGui::PopItemWidth();

    if (ImGui::Button("set material")) {
        ImGui::OpenPopup("select_material_popup");
    }

    ImGui::SameLine();
    if (ImGui::BeginPopup("select_material_popup")) {
        for (ResourceID const & mat_id : man.material_ids()) {
            Material const & material = man.get_material(mat_id);
            if (ImGui::Selectable(material.name.c_str())) {
                if (component.resource_id() != mat_id) {
                    context.editor().perform_action<ActionSetResource<
                        MaterialComponent
                    > >(
                        id,
                        mat_id
                    );
                }
            }
        }
        ImGui::EndPopup();
    }
}

} // namespace prt3

#endif
