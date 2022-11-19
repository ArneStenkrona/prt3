#ifndef PRT3_EDITOR_H
#define PRT3_EDITOR_H

#include "src/engine/core/context.h"
#include "src/engine/editor/editor_camera.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/node.h"
#include "src/engine/editor/gizmo/gizmo_manager.h"
#include "src/engine/editor/action/action_manager.h"


namespace prt3 {

class Editor {
public:
    Editor(Context & context);

    void update(float delta_time);

    Camera & get_camera() { return m_camera.get_camera(); }

    NodeID selected_node() const
    { return m_editor_context.get_selected_node(); }

    ActionManager & action_manager() { return m_action_manager; }

    template<typename ActionType, typename... ArgTypes>
    bool perform_action(ArgTypes... args)
    { return m_action_manager.perform<ActionType>(args...); }

private:
    Context & m_context;
    EditorCamera m_camera;

    EditorContext m_editor_context;
    GizmoManager m_gizmo_manager;
    ActionManager m_action_manager;
};

} // namespace prt3

#endif
