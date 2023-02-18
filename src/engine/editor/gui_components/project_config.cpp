#include "project_config.h"

#include "src/engine/editor/gui_components/panel.h"
#include "src/util/fixed_string.h"

using namespace prt3;

void main_scene(EditorContext & context) {
    begin_group_panel("main scene");

    Project & project = context.project();

    thread_local bool set_main_scene_open = false;
    if (ImGui::SmallButton("set")) {
        set_main_scene_open = true;
    }

    ImGui::SameLine();

    ImGui::InputText(
        "path",
        const_cast<char *>(project.main_scene_path().c_str()),
        project.main_scene_path().length(),
        ImGuiInputTextFlags_ReadOnly
    );

    if (set_main_scene_open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("set main scene");

        if (file_dialog.showFileDialog("set main scene",
            imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(0, 0))
        ) {
            std::string const & path = file_dialog.selected_path;
            project.set_main_scene_path(path);
        }
        set_main_scene_open = ImGui::IsPopupOpen(ImGui::GetID("set main scene"), 0);
    }

    end_group_panel();
}

void autoload_scripts(EditorContext & context) {
    begin_group_panel("autoloaded scripts");

    Project & project = context.project();

    std::unordered_set<UUID> const & autoload_scripts = project.autoload_scripts();

    ImGui::PushItemWidth(160);

    UUID to_remove = NULL;

    static std::unordered_set<UUID> uuids;
    uuids.clear();

    for (UUID const & uuid : autoload_scripts) {
        ImGui::PushID(
            static_cast<int>(uuid)
        );

        ImGui::Text("%s", Script::get_script_name(uuid));

        ImGui::SameLine();

        if (ImGui::SmallButton("remove")) {
            to_remove = uuid;
        } else {
            uuids.insert(uuid);
        }

        ImGui::PopID();

    }
    if (to_remove != NULL) {
        project.remove_autoload_script(to_remove);
    }

    thread_local std::vector<UUID> available_uuids;
    available_uuids.resize(0);
    thread_local std::vector<const char *> available_scripts;
    available_scripts.resize(0);

    for (auto const & pair : Script::script_names()) {
        if (uuids.find(pair.first) == uuids.end()) {
            available_uuids.push_back(pair.first);
            available_scripts.push_back(pair.second);
        }
    }

    if (ImGui::SmallButton("add script")) {
        ImGui::OpenPopup("select_script_popup");
    }

    ImGui::SameLine();
    if (ImGui::BeginPopup("select_script_popup")) {
        for (size_t i = 0; i < available_uuids.size(); ++i) {
            if (ImGui::Selectable(available_scripts[i])) {
                project.add_autoload_script(available_uuids[i]);
            }
        }
        ImGui::EndPopup();
    }

    ImGui::PopItemWidth();

    end_group_panel();
}

void prt3::project_config(EditorContext & context, bool & open) {
    ImGui::SetNextWindowSize(ImVec2(430, 450), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("project config", &open, ImGuiWindowFlags_NoDocking))
    {
        ImGui::End();
        return;
    }

    main_scene(context);

    autoload_scripts(context);

    ImGui::End();
}
