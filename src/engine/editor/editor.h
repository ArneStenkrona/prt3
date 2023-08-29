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
    ~Editor();

    void update(float delta_time);

    Camera & get_camera() { return m_camera.get_camera(); }

    ActionManager & action_manager() { return m_action_manager; }

    template<typename ActionType, typename... ArgTypes>
    bool perform_action(ArgTypes &&... args)
    { return m_action_manager.perform<ActionType>(args...); }

    void collect_render_data(
        EditorRenderData & data
    );

private:
    Context & m_context;
    EditorCamera m_camera;

    EditorContext m_editor_context;
    GizmoManager m_gizmo_manager;
    ActionManager m_action_manager;

    ResourceID m_line_box_res_id;
};

} // namespace prt3

#endif
