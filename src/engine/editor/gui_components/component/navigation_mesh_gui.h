#ifndef PRT3_NAVIGATION_MESH_GUI_H
#define PRT3_NAVIGATION_MESH_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/component/navigation_mesh.h"
#include "src/engine/physics/collider.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<NavigationMeshComponent>(
    EditorContext & context,
    NodeID id
) {
    // NavigationMeshComponent & comp =
    //     context.scene().get_component<NavigationMeshComponent>(id);

    struct NavMeshParams {
        CollisionLayer layer = 0;
        float granularity = 1.0f;
        float max_edge_deviation = 1.0f;
        float max_edge_length = 1.0f;
        float min_width = 1.0f;
        float min_height = 1.0f;
    };

    thread_local NavMeshParams params;

    static char const * err_msg = "Failed to generate navigation mesh.";

    static double error_deadline = 0.0;

    if (ImGui::Button("generate")) {
        ImGui::OpenPopup("generate_nav_mesh");
    }

    if (ImGui::BeginPopup("generate_nav_mesh")) {

        bit_field("layer", &params.layer);

        ImGui::DragFloat(
            "cell size",
            &params.granularity,
            0.1f,
            0.1f,
            5.0f,
            "%.1f"
        );

        ImGui::DragFloat(
            "max edge deviation",
            &params.max_edge_deviation,
            0.1f,
            0.1f,
            5.0f,
            "%.1f"
        );

        ImGui::DragFloat(
            "max edge length",
            &params.max_edge_length,
            0.1f,
            0.1f,
            5.0f,
            "%.1f"
        );

        ImGui::DragFloat(
            "min width",
            &params.min_width,
            0.1f,
            0.1f,
            5.0f,
            "%.1f"
        );

        ImGui::DragFloat(
            "min height",
            &params.min_height,
            0.1f,
            0.1f,
            5.0f,
            "%.1f"
        );

        if (ImGui::Button("generate")) {
            NavigationSystem & sys = context.scene().navigation_system();
            NavMeshID nav_mesh_id = sys.generate_nav_mesh(
                context.scene(),
                params.layer,
                params.granularity,
                params.max_edge_deviation,
                params.max_edge_length,
                params.min_height,
                params.min_width
            );
            if (nav_mesh_id != NO_NAV_MESH) {
                context.editor().perform_action<ActionSetField<
                    NavigationMeshComponent, NavMeshID>
                >(
                    id,
                    offsetof(NavigationMeshComponent, m_nav_mesh_id),
                    nav_mesh_id
                );
            } else {
                error_deadline = ImGui::GetTime() + 10.0;
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::GetTime() < error_deadline) {
        ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "%s", err_msg);
    }

}

} // namespace prt3

#endif
