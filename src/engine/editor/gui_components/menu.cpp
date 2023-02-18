#include "menu.h"

#include "src/engine/editor/gui_components/project_config.h"
#include "src/engine/editor/editor.h"
#include "src/util/file_util.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include <iostream>
#include <fstream>

using namespace prt3;

void save_scene(Scene const & scene, std::string const & path) {
    std::ofstream out(path, std::ios::binary);

    scene.serialize(out);

    out.close();

    emscripten_save_file_via_put(path);
}

void load_scene(EditorContext & context, std::string const & path) {
    std::ifstream in(path, std::ios::binary);

    context.scene().deserialize(in);

    in.close();

    context.editor().action_manager().clear();
    context.set_selected_node(context.scene().get_root_id());
}

void save_project(Project & project, std::string const & path) {
    std::ofstream out(path, std::ios::binary);

    project.serialize(out);

    out.close();

    emscripten_save_file_via_put(path);
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
    if (ImGui::BeginMenu("file")) {
        if (ImGui::MenuItem("save project")) {
            save_project_open = true;
        }

        if (ImGui::MenuItem("load project")) {
            load_project_open = true;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("save scene")) {
            save_scene_open = true;
        }

        if (ImGui::MenuItem("load scene")) {
            load_scene_open = true;
        }

        ImGui::EndMenu();
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
            save_scene(context.scene(), path);
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
