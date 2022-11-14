#include "menu.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

using namespace prt3;

void prt3::menu(EditorContext & /*context*/) {
    // Menu Bar
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("file")) {
        if (ImGui::MenuItem("save")) {
            // TODO: implement
        }

        if (ImGui::MenuItem("load")) {
            // TODO: implement
        }

        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}
