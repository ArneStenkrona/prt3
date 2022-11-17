#include "gizmo_manager.h"

#include "src/engine/editor/editor_context.h"

using namespace prt3;

GizmoManager::GizmoManager(
    EditorContext & editor_context,
    Camera & camera
) : m_editor_context{editor_context},
    m_camera{camera}
{}

bool GizmoManager::update() {
    if (!ImGui::IsAnyItemActive()) {
        update_input();
    }

    bool manipulated = false;

    ImGuizmo::BeginFrame();

    glm::mat4 view = m_camera.get_view_matrix();
    glm::mat4 projection = m_camera.get_projection_matrix();
    glm::mat4 identity{1.0f};

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    Scene & scene = m_editor_context.scene();

    NodeID selected_node = m_editor_context.get_selected_node();

    if (selected_node != NO_NODE) {
        Node & node = scene.get_node(selected_node);

        glm::mat4 transform = node.get_global_transform(scene).to_matrix();
        manipulated = ImGuizmo::Manipulate(
            &view[0][0],
            &projection[0][0],
            m_operation,
            ImGuizmo::WORLD,
            &transform[0][0],
            nullptr,
            nullptr
        );
        node.set_global_transform(scene, Transform().from_matrix(transform));
    }

    return manipulated;
}

void GizmoManager::update_input() {
    Input const & input = m_editor_context.context().input();

    if (input.get_key(KEY_CODE_LSHIFT)) {
        if (input.get_key_down(KEY_CODE_G)) {
            m_operation = ImGuizmo::TRANSLATE;
        }
        if (input.get_key_down(KEY_CODE_R)) {
            m_operation = ImGuizmo::ROTATE;
        }
        if (input.get_key_down(KEY_CODE_S)) {
            m_operation = ImGuizmo::SCALE;
        }
    }
}
