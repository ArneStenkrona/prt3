#include "component_gui_utility.h"

using namespace prt3;

char const * prt3::texture_dialogue(
    EditorContext & context,
    ResourceID tex_id
) {
    Scene & scene = context.scene();
    TextureManager & man = context.get_texture_manager();

    ImGui::PushItemWidth(160);

    if (tex_id != NO_RESOURCE) {
        thread_local std::string tex_path;
        tex_path = man.get_texture_path(tex_id);
        ImGui::InputText(
            "path",
            tex_path.data(),
            tex_path.size(),
            ImGuiInputTextFlags_ReadOnly
        );
        if (ImGui::Button("clear")) {
            return "";
        }
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
    char const * path = nullptr;
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
                        path = file_dialog.selected_path.c_str();
                    }
                    man.free_texture_ref(new_res_id);
                }
            }
            open = ImGui::IsPopupOpen(ImGui::GetID("import texture"), 0);
        }

        if (ImGui::GetTime() < error_deadline) {
            ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "%s", err_msg);
        }

        ImGui::EndPopup();
    }

    return path;
}
