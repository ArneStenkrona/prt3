#ifndef PRT3_MODEL_GUI_COMMON_H
#define PRT3_MODEL_GUI_COMMON_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/model.h"
#include "src/engine/rendering/model_manager.h"

#include "imgui.h"

namespace prt3 {

template<typename ModelComponentType>
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
            scene.get_component<ModelComponentType>(m_node_id).model_handle();
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<ModelComponentType>(m_node_id)
            .set_model_handle(scene, m_model_handle);
        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<ModelComponentType>(m_node_id)
            .set_model_handle(scene, m_original_model_handle);
        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    ModelHandle m_model_handle;
    ModelHandle m_original_model_handle;
};

template<typename ModelComponentType>
void show_model_component(
    EditorContext & context,
    NodeID id,
    bool require_animated
) {
    Scene & scene = context.scene();
    ModelManager & man = context.get_model_manager();

    ModelComponentType & component = scene.get_component<ModelComponentType>(id);
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

    static char const * const err_import = "Failed to import model";
    static char const * const err_animated = "Model does not contain any animations";
    static char const * err_msg = err_import;

    static bool open = false;
    static double error_deadline = 0.0;
    if (ImGui::BeginPopup("select_model_popup")) {
        if (ImGui::Button("import from file")) {
            open = true;
            error_deadline = 0.0;
        }

        if (open) {
            auto & file_dialog = context.file_dialog();
            ImGui::OpenPopup("import model");

            if (file_dialog.showFileDialog(
                    "import model",
                    imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                    ImVec2(1400, 700)
            )) {
                ModelHandle new_handle =
                    man.upload_model(file_dialog.selected_path.c_str());
                if (new_handle == NO_MODEL) {
                    // failed to load
                    error_deadline = ImGui::GetTime() + 10.0;
                    err_msg = err_import;
                } else {
                    if (!require_animated ||
                        man.get_model(new_handle).is_animated()) {
                        if (new_handle != handle) {
                            context.editor()
                                .perform_action<ActionSetModel<ModelComponentType> >(
                                id,
                                new_handle
                            );
                        }
                    } else {
                        error_deadline = ImGui::GetTime() + 10.0;
                        err_msg = err_animated;
                    }
                }
            }
            open = ImGui::IsPopupOpen(ImGui::GetID("import model"), 0);
        }

        if (ImGui::GetTime() < error_deadline) {
            ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "%s", err_msg);
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("imported models")) {
            ModelHandle current = 0;
            for (Model const & model : man.models()) {
                if (!require_animated || model.is_animated()) {
                    if (ImGui::MenuItem(model.name().c_str())) {
                        if (current != handle) {
                            context.editor()
                                .perform_action<ActionSetModel<ModelComponentType> >(
                                id,
                                current
                            );
                        }
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
