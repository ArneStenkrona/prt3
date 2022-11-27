#ifndef PRT3_MODEL_GUI_H
#define PRT3_MODEL_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/model.h"
#include "src/engine/rendering/model_manager.h"

#include "imgui.h"

namespace prt3 {

class ActionSetModel : public Action {
public:
    ActionSetModel(
        EditorContext & editor_context,
        NodeID node_id,
        ModelHandle model_handle
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_model_handle{model_handle}
    {
        Scene const & scene = m_editor_context->scene();

        m_original_model_handle =
            scene.get_component<ModelComponent>(m_node_id).model_handle();
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<ModelComponent>(m_node_id)
            .set_model_handle(m_model_handle);
        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<ModelComponent>(m_node_id)
            .set_model_handle(m_original_model_handle);
        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    ModelHandle m_model_handle;
    ModelHandle m_original_model_handle;
};

template<>
void inner_show_component<ModelComponent>(
    EditorContext & context,
    NodeID id
) {
    Scene & scene = context.scene();
    ModelManager & man = context.get_model_manager();

    ModelComponent & component = scene.get_component<ModelComponent>(id);
    ModelHandle handle = component.model_handle();

    ImGui::PushItemWidth(160);

    if (handle != NO_RESOURCE) {
        Model const & model = man.get_model(handle);

        static FixedString<64> model_path;
        model_path = model.path().c_str();
        ImGui::InputText(
            "path",
            model_path.data(),
            model_path.buf_size(),
            ImGuiInputTextFlags_ReadOnly
        );

        static FixedString<64> name;
        name = model.name().c_str();
        ImGui::InputText(
            "name",
            name.data(),
            name.buf_size(),
            ImGuiInputTextFlags_ReadOnly
        );
    } else {
        ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "%s", "no model");
    }

    ImGui::PopItemWidth();

    if (ImGui::Button("set model")) {
        ImGui::OpenPopup("select_model_popup");
    }

    static bool open = false;
    static double errorDeadline = 0.0;
    if (ImGui::BeginPopup("select_model_popup")) {
        if (ImGui::Button("import from file")) {
            open = true;
            errorDeadline = 0.0;
        }

        if (open) {
            auto & file_dialog = context.file_dialog();
            ImGui::OpenPopup("import model");

            if (file_dialog.showFileDialog("import model", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(1400, 700))) {
                ModelHandle new_handle = man.upload_model(file_dialog.selected_path.c_str());
                if (new_handle == NO_MODEL) {
                    // failed to load
                    errorDeadline =  ImGui::GetTime() + 10.0;
                } else {
                    if (new_handle != handle) {
                        context.editor().perform_action<ActionSetModel>(
                            id,
                            new_handle
                        );
                    }
                }
            }
            open = ImGui::IsPopupOpen(ImGui::GetID("import model"), 0);
        }

        if (ImGui::GetTime() < errorDeadline) {
            ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "%s", "Failed to import model");
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("imported models")) {
            ModelHandle current = 0;
            for (Model const & model : man.models()) {
                if (ImGui::MenuItem(model.name().c_str())) {
                    if (current != handle) {
                        context.editor().perform_action<ActionSetModel>(
                            id,
                            current
                        );
                    }
                }
                ++current;
            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}

} // namespace prt3

#endif
