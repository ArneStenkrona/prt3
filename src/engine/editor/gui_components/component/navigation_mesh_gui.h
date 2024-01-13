#ifndef PRT3_NAVIGATION_MESH_GUI_H
#define PRT3_NAVIGATION_MESH_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/component/navigation_mesh.h"
#include "src/engine/physics/collider.h"

#include "imgui.h"

namespace prt3 {

struct NavMeshParams {
    CollisionLayer layer = 0;
    float granularity = 1.0f;
    float max_edge_deviation = 1.0f;
    float max_edge_length = 1.0f;
    float min_width = 1.0f;
    float min_height = 1.0f;
};

class ActionSetNavMesh : public Action {
public:
    ActionSetNavMesh(
        EditorContext & editor_context,
        NodeID node_id,
        NavMeshParams const & params
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_params{params}
    {
        Scene const & scene = m_editor_context->scene();

        std::stringstream stream;
        NavigationMeshComponent const & comp =
            scene.get_component<NavigationMeshComponent>(m_node_id);
        comp.serialize(stream, scene);

        std::string const & s = stream.str();
        m_original_data.reserve(s.size());
        m_original_data.assign(s.begin(), s.end());
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        NavigationSystem & sys = scene.navigation_system();

        NavigationMeshComponent & comp =
            scene.get_component<NavigationMeshComponent>(m_node_id);

        if (m_new_data.empty()) {
            if (m_nav_mesh_id != NO_NAV_MESH) {
                sys.remove_nav_mesh(m_nav_mesh_id);
            }

            m_nav_mesh_id = sys.generate_nav_mesh(
                scene,
                m_params.layer,
                m_params.granularity,
                m_params.max_edge_deviation,
                m_params.max_edge_length,
                m_params.min_width,
                m_params.min_height
            );

            if (m_nav_mesh_id == NO_NAV_MESH) {
                imemstream in(m_original_data.data(), m_original_data.size());
                comp.deserialize(in, scene);
                return false;
            }

            comp.m_nav_mesh_id = m_nav_mesh_id;

            std::stringstream stream;
            comp.serialize(stream, scene);

            std::string const & s = stream.str();
            m_new_data.reserve(s.size());
            m_new_data.assign(s.begin(), s.end());
        } else {
            imemstream in(m_new_data.data(), m_new_data.size());

            comp.deserialize(in, scene);
        }

        return true;
    }

    virtual bool unapply() {
        imemstream in(m_original_data.data(), m_original_data.size());
        Scene & scene = m_editor_context->scene();

        if (m_nav_mesh_id != NO_NAV_MESH) {
            NavigationSystem & sys = scene.navigation_system();
            sys.remove_nav_mesh(m_nav_mesh_id);
        }

        NavigationMeshComponent & comp =
            scene.get_component<NavigationMeshComponent>(m_node_id);
        comp.deserialize(in, scene);

        m_nav_mesh_id = comp.nav_mesh_id();

        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    NavMeshID m_nav_mesh_id = NO_NAV_MESH;

    std::vector<char> m_original_data;
    std::vector<char> m_new_data;

    NavMeshParams m_params;
};

template<>
void inner_show_component<NavigationMeshComponent>(
    EditorContext & context,
    NodeID id
) {
    NavigationMeshComponent & comp =
        context.scene().get_component<NavigationMeshComponent>(id);

    if (comp.nav_mesh_id() == NO_NAV_MESH) {
        ImGui::Text("No navigation mesh");
    }

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
            if (!context.editor().perform_action<ActionSetNavMesh>(
                    id,
                    params
                )
            ) {
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
