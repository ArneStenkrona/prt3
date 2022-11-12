#include "editor_gui.h"

#include "src/engine/editor/gui_components/scene_hierarchy.h"
#include "src/engine/editor/gui_components/node_inspector.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include <iostream>

void prt3::editor_gui(EditorContext & context) {
    static ImGuiDockNodeFlags dockspace_flags =
        ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar |
                                    ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar            |
                    ImGuiWindowFlags_NoCollapse            |
                    ImGuiWindowFlags_NoResize              |
                    ImGuiWindowFlags_NoMove                |
                    ImGuiWindowFlags_NoBringToFrontOnFocus |
                    ImGuiWindowFlags_NoNavFocus            |
                    ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);

    ImGui::PopStyleVar();
    ImGui::PopStyleVar(2);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("PRT3 Editor");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        static auto first_time = true;
        if (first_time)
        {
            first_time = false;

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
            auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);

            ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
            ImGui::DockBuilderDockWindow("Scene", dock_id_left);
            ImGui::DockBuilderFinish(dockspace_id);
        }
    }

    ImGui::End();

    ImGui::Begin("Scene");
    scene_hierarchy(context);
    ImGui::End();

    ImGui::Begin("Inspector");
    node_inspector(context);
    ImGui::End();

    context.commit_frame();
}
