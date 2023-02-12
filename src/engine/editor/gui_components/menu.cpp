#include "menu.h"

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

void load_scene(Scene & scene, std::string const & path) {
    std::ifstream in(path, std::ios::binary);

    scene.deserialize(in);

    in.close();
}

void prt3::menu(EditorContext & context) {
    // Menu Bar
    ImGui::BeginMainMenuBar();

    thread_local bool save_open = false;
    thread_local bool load_open = false;

    if (ImGui::BeginMenu("file")) {

        if (ImGui::MenuItem("save scene")) {
            save_open = true;
        }

        if (ImGui::MenuItem("load scene")) {
            load_open = true;
        }

        ImGui::EndMenu();
    }

    if (save_open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("save scene");

        if (file_dialog.showFileDialog("save scene",
            imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(0, 0))
        ) {
            std::string const & path = file_dialog.selected_path;
            save_scene(context.scene(), path);
        }
        save_open = ImGui::IsPopupOpen(ImGui::GetID("save scene"), 0);
    }

    if (load_open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("load scene");

        if (file_dialog.showFileDialog("load scene",
            imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(0, 0))
        ) {
            std::string const & path = file_dialog.selected_path;
            load_scene(context.scene(), path);
            context.editor().action_manager().clear();
            context.set_selected_node(context.scene().get_root_id());
        }
        load_open = ImGui::IsPopupOpen(ImGui::GetID("load scene"), 0);
    }

    ImGui::EndMainMenuBar();
}
