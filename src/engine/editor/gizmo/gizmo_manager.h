#ifndef PRT3_GIZMO_MANAGER_H
#define PRT3_GIZMO_MANAGER_H

#include "imgui.h"
#include "ImGuizmo.h"

#include "src/engine/rendering/camera.h"

namespace prt3 {

class EditorContext;

class GizmoManager {
public:
    GizmoManager(EditorContext & editor_context, Camera & m_camera);

    bool update();

private:
    EditorContext & m_editor_context;
    Camera & m_camera;

    ImGuizmo::OPERATION m_operation = ImGuizmo::TRANSLATE;

    void update_input();
};

} // namespace prt3

#endif
