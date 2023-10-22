#include "menu.h"

#include "src/engine/editor/gui_components/project_config.h"
#include "src/engine/editor/editor.h"
#include "src/util/file_util.h"

#include "src/daedalus/map/map.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include <iostream>
#include <fstream>

using namespace prt3;

void new_scene(EditorContext & context) {
    context.scene().clear();

    context.editor().action_manager().clear();
    context.editor().action_manager().reset_action_count();
    context.set_selected_node(context.scene().get_root_id());
}

void save_scene(EditorContext & context, std::string const & path) {
    std::ofstream out(path, std::ios::binary);

    context.scene().serialize(out);
    context.set_save_point();

    out.close();

#ifdef __EMSCRIPTEN__
    emscripten_save_file_via_put(path);
#endif // __EMSCRIPTEN__
}

void load_scene(EditorContext & context, std::string const & path) {
    std::ifstream in(path, std::ios::binary);

    context.scene().deserialize(in);

    in.close();

    context.editor().action_manager().clear();
    context.editor().action_manager().reset_action_count();
    context.set_selected_node(context.scene().get_root_id());
}

void save_project(Project & project, std::string const & path) {
    std::ofstream out(path, std::ios::binary);

    project.serialize(out);

    out.close();

#ifdef __EMSCRIPTEN__
    emscripten_save_file_via_put(path);
#endif // __EMSCRIPTEN__
}

void load_project(EditorContext & context, std::string const & path) {
    std::ifstream in(path, std::ios::binary);

    context.project().deserialize(in);

    in.close();

    load_scene(context, context.project().main_scene_path());
}

void prt3::menu(EditorContext & context) {
    // Menu Bar
    ImGui::BeginMainMenuBar();

    /* file */
    thread_local bool save_project_open = false;
    thread_local bool load_project_open = false;

    thread_local bool save_scene_open = false;
    thread_local bool load_scene_open = false;

    thread_local bool import_map_open = false;

    enum UnsavedAction {
        unsaved_action_new_scene,
        unsaved_action_load_project,
        unsaved_action_load_scene
    };

    bool show_unsaved = false;
    thread_local UnsavedAction unsaved_action{};

    if (!context.is_saved()) {
        ImGui::TextUnformatted("*");
    }

    if (ImGui::BeginMenu("file")) {
        if (ImGui::MenuItem("save project")) {
            save_project_open = true;
        }

        if (ImGui::MenuItem("load project")) {
            if (context.is_saved()) {
                load_project_open = true;
            } else {
                show_unsaved = true;
                unsaved_action = unsaved_action_load_project;
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("new scene")) {
            if (context.is_saved()) {
                new_scene(context);
            } else {
                show_unsaved = true;
                unsaved_action = unsaved_action_new_scene;
            }
        }

        if (ImGui::MenuItem("save scene")) {
            save_scene_open = true;
        }

        if (ImGui::MenuItem("load scene")) {
            if (context.is_saved()) {
                load_scene_open = true;
            } else {
                show_unsaved = true;
                unsaved_action = unsaved_action_load_scene;
            }
        }

        ImGui::Separator();

        /* map */
        if (ImGui::MenuItem("import map")) {
            import_map_open = true;
        }

        ImGui::EndMenu();
    }

    if (show_unsaved) {
        ImGui::OpenPopup("unsaved changes");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopup("unsaved changes")) {
        ImGui::TextUnformatted(
            "You have unsaved changes.\nAre you sure you want to go ahead?"
        );
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            switch (unsaved_action) {
                case unsaved_action_new_scene: new_scene(context); break;
                case unsaved_action_load_project: load_project_open = true; break;
                case unsaved_action_load_scene: load_scene_open = true; break;
            }
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (save_project_open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("save project");

        if (file_dialog.showFileDialog("save project",
            imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(0, 0))
        ) {
            std::string const & path = file_dialog.selected_path;
            save_project(context.project(), path);
        }
        save_project_open = ImGui::IsPopupOpen(ImGui::GetID("save project"), 0);
    }

    if (load_project_open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("load project");

        if (file_dialog.showFileDialog("load project",
            imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(0, 0))
        ) {
            std::string const & path = file_dialog.selected_path;
            load_project(context, path);
        }
        load_project_open = ImGui::IsPopupOpen(ImGui::GetID("load project"), 0);
    }

    if (save_scene_open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("save scene");

        if (file_dialog.showFileDialog("save scene",
            imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(0, 0))
        ) {
            std::string const & path = file_dialog.selected_path;
            save_scene(context, path);
        }
        save_scene_open = ImGui::IsPopupOpen(ImGui::GetID("save scene"), 0);
    }

    if (load_scene_open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("load scene");

        if (file_dialog.showFileDialog("load scene",
            imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(0, 0))
        ) {
            std::string const & path = file_dialog.selected_path;
            load_scene(context, path);
        }
        load_scene_open = ImGui::IsPopupOpen(ImGui::GetID("load scene"), 0);
    }

    if (import_map_open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("import map");

        if (file_dialog.showFileDialog("import map",
            imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(0, 0))
        ) {
            std::string const & path = file_dialog.selected_path;
            dds::Map map = dds::Map::parse_map_from_model(path.c_str());
            std::string map_out_path =
                path.substr(0, path.find_last_of('.')) + ".map";

            std::ofstream map_out{map_out_path, std::ios::binary};
            map.serialize(map_out);
            map_out.close();
#ifdef __EMSCRIPTEN__
            emscripten_save_file_via_put(map_out_path);
#endif // __EMSCRIPTEN__
        }
        import_map_open = ImGui::IsPopupOpen(ImGui::GetID("import map"), 0);
    }

    /* project */
    thread_local bool project_config_open = false;

    if (ImGui::BeginMenu("project")) {
        if (ImGui::MenuItem("config")) {
            project_config_open = true;
        }

        ImGui::EndMenu();
    }

    if (project_config_open) {
        project_config(context, project_config_open);
    }

    ImGui::EndMainMenuBar();
}
