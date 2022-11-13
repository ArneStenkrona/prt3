#include "menu.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

using namespace prt3;

void prt3::menu(EditorContext & context) {
    // Menu Bar
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save")) {
            // TODO: implement
        }

        if (ImGui::MenuItem("Load")) {
            // TODO: implement
        }

        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}
