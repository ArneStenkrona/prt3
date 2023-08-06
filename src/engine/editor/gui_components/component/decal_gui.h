#ifndef PRT3_DECAL_GUI_H
#define PRT3_DECAL_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/gui_components/component/component_gui_utility.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/decal.h"

#include "imgui.h"

#include <string>

namespace prt3 {

class ActionSetDecalTexture : public Action {
public:
    ActionSetDecalTexture(
        EditorContext & editor_context,
        NodeID node_id,
        std::string const & texture
    ) : m_editor_context{&editor_context},
        m_node_id{node_id}
    {
        Scene const & scene = editor_context.scene();
        TextureManager & man = m_editor_context->get_texture_manager();
        ResourceID orig_id = scene.get_component<Decal>(node_id).texture_id();
        m_id_to_free = orig_id;

        m_path = texture;
        if (orig_id != NO_RESOURCE) {
            m_original_path = man.get_texture_path(orig_id);
        }
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        TextureManager & man = m_editor_context->get_texture_manager();

        if (m_id_to_free != NO_RESOURCE) {
            man.free_texture_ref(m_id_to_free);
        }

        ResourceID res_id = scene.upload_texture(m_path);
        scene.get_component<Decal>(m_node_id).texture_id() = res_id;

        m_id_to_free = res_id;

        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        TextureManager & man = m_editor_context->get_texture_manager();

        man.free_texture_ref(m_id_to_free);

        if (!m_original_path.empty()) {
            ResourceID res_id = scene.upload_texture(m_original_path);
            scene.get_component<Decal>(m_node_id).texture_id() = res_id;
            m_id_to_free = res_id;
        } else {
            m_id_to_free = NO_RESOURCE;
            scene.get_component<Decal>(m_node_id).texture_id() = NO_RESOURCE;
        }

        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;
    ResourceID m_id_to_free;
    std::string m_path;
    std::string m_original_path;
};

template<>
void inner_show_component<Decal>(
    EditorContext & context,
    NodeID id
) {
    ImGui::PushItemWidth(125);

    Scene & scene = context.scene();
    TextureManager & man = context.get_texture_manager();

    Decal & comp = scene.get_component<Decal>(id);

    ImGui::PushItemWidth(160);

    ResourceID tex_id = comp.texture_id();

    if (tex_id != NO_RESOURCE) {

        thread_local std::string tex_path;
        tex_path = man.get_texture_path(tex_id);
        ImGui::InputText(
            "path",
            tex_path.data(),
            tex_path.size(),
            ImGuiInputTextFlags_ReadOnly
        );
    } else {
        ImGui::TextColored(ImVec4(1.0f,1.0f,0.0f,1.0f), "%s", "no texture");
    }

    ImGui::PopItemWidth();

    if (ImGui::Button("set texture")) {
        ImGui::OpenPopup("select_texture_popup");
    }

    static char const * const err_import = "Failed to import texture";
    static char const * err_msg = err_import;

    static bool open = false;
    static double error_deadline = 0.0;
    if (ImGui::BeginPopup("select_texture_popup")) {
        if (ImGui::Button("import from file")) {
            open = true;
            error_deadline = 0.0;
        }

        if (open) {
            auto & file_dialog = context.file_dialog();
            ImGui::OpenPopup("import texture");

            if (file_dialog.showFileDialog(
                    "import texture",
                    imgui_addons::ImGuiFileBrowser::DialogMode::OPEN,
                    ImVec2(0, 0)
            )) {
                ResourceID new_res_id =
                    scene.upload_texture(file_dialog.selected_path.c_str());
                if (new_res_id == NO_RESOURCE) {
                    // failed to load
                    error_deadline = ImGui::GetTime() + 10.0;
                    err_msg = err_import;
                } else {
                    if (new_res_id != tex_id) {
                        context.editor()
                            .perform_action<ActionSetDecalTexture>(
                            id,
                            file_dialog.selected_path.c_str()
                        );
                        man.free_texture_ref(new_res_id);
                    }
                }
            }
            open = ImGui::IsPopupOpen(ImGui::GetID("import texture"), 0);
        }

        if (ImGui::GetTime() < error_deadline) {
            ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "%s", err_msg);
        }

        ImGui::EndPopup();
    }

    edit_field<Decal, glm::vec3>(
        context,
        id,
        "dimension",
        offsetof(Decal, m_dimensions)
    );

    ImGui::PopItemWidth();
}

} // namespace prt3

#endif // PRT3_DECAL_H
