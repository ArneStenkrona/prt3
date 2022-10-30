#ifndef PRT3_EDITOR_H
#define PRT3_EDITOR_H

#include "src/engine/core/context.h"
#include "src/engine/editor/editor_camera.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/node.h"

namespace prt3 {

class Editor {
public:
    Editor(Context & context);

    void update(float delta_time);

    Camera & get_camera() { return m_camera.get_camera(); }

    NodeID selected_node() const
    { return m_editor_context.get_selected_node(); }
private:
    Context & m_context;
    EditorCamera m_camera;

    EditorContext m_editor_context;
};

} // namespace prt3

#endif
