#ifndef PRT3_ANIMATED_MESH_GUI_H
#define PRT3_ANIMATED_MESH_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/gui_components/component/mesh_gui_common.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/animated_mesh.h"

#include "imgui.h"

#include <string>

namespace prt3 {

class ActionSetArmatureID : public Action {
public:
    ActionSetArmatureID(
        EditorContext & editor_context,
        NodeID node_id,
        NodeID armature_id
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_armature_id{armature_id}
    {
        Scene const & scene = editor_context.scene();
        m_original_armature_id = scene.get_component<AnimatedMesh>(node_id).armature_id();
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<AnimatedMesh>(m_node_id).set_armature_id(m_armature_id);

        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<AnimatedMesh>(m_node_id).set_armature_id(m_original_armature_id);

        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;
    NodeID m_armature_id;
    NodeID m_original_armature_id;
};

template<>
void inner_show_component<AnimatedMesh>(
    EditorContext & context,
    NodeID id
) {
    ImGui::NewLine();

    show_mesh_component<AnimatedMesh>(context, id);

    Scene const & scene = context.scene();
    AnimatedMesh const & component = scene.get_component<AnimatedMesh>(id);
    NodeID armature_id = component.armature_id();

    ModelManager const & man = scene.model_manager();
    ModelHandle model_handle = component.resource_id() == NO_RESOURCE ?
        component.resource_id() :
        man.get_model_handle_from_mesh_id(component.resource_id());

    thread_local std::string armature_path;
    thread_local std::string error;
    error = "";

    if (armature_id == NO_NODE) {
        armature_path = "none";
    } else if (!scene.node_exists(armature_id)) {
        armature_path = "missing node";
        error = "referenced armature node does not exist";
    } else if (!scene.has_component<Armature>(armature_id)) {
        armature_path = scene.get_node_path(component.armature_id());
        error = "referenced node has no armature component";
    } else if (scene.get_component<Armature>(armature_id).model_handle() != model_handle) {
        armature_path = scene.get_node_path(component.armature_id());
        error = "armature is from the wrong model";
    } else {
        armature_path = scene.get_node_path(component.armature_id());
    }

    ImGui::InputText(
        "armature",
        armature_path.data(),
        armature_path.length(),
        ImGuiInputTextFlags_ReadOnly
    );

    if (error != "") {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "error");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", error.c_str());
        }
    }

    if (ImGui::Button("set armature")) {
        ImGui::OpenPopup("select_armature_popup");
    }

    if (ImGui::BeginPopup("select_armature_popup")) {
        if (ImGui::BeginMenu("armatures")) {
            auto const & armatures = scene.get_all_components<Armature>();

            for (Armature const & armature : armatures) {
                if (armature.model_handle() == model_handle) {
                    NodeID a_id = armature.node_id();
                    if (ImGui::MenuItem(scene.get_node_path(a_id).c_str())) {
                        if (a_id != component.armature_id()) {
                            context.editor()
                                .perform_action<ActionSetArmatureID>(
                                id,
                                a_id
                            );
                        }
                    }
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}

} // namespace prt3

#endif
