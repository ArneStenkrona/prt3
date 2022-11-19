#include "editor.h"

#include "src/engine/editor/editor_context.h"
#include "src/engine/editor/gui_components/editor_gui.h"
#include "src/engine/scene/node.h"

#include "imgui.h"

#include <iostream>

using namespace prt3;

Editor::Editor(Context & context)
 : m_context{context},
   m_camera{context.renderer().window_width(),
            context.renderer().window_height()},
   m_editor_context{*this, m_context},
   m_gizmo_manager{m_editor_context, m_camera.get_camera()},
   m_action_manager{m_editor_context} {
}

void Editor::update(float delta_time) {
    Input const & input = m_context.input();

    ImGui::NewFrame();

    editor_gui(m_editor_context);
    m_gizmo_manager.update();

    if (input.get_key(KEY_CODE_LCTRL)){
        if (input.get_key_down(KEY_CODE_Z)) {
            if (input.get_key(KEY_CODE_LSHIFT)) {
                m_action_manager.redo();
            } else {
                m_action_manager.undo();
            }
        }
    }

    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) &&
        !ImGuizmo::IsUsing()) {
        m_camera.update(delta_time, input);

        if (input.get_key_down(KEY_CODE_MOUSE_RIGHT)) {
            int x, y;
            input.get_cursor_position(x, y);

            NodeID id = m_context.renderer().get_selected(x, y);
            m_editor_context.set_selected_node(id);
        }
    }

    ImGui::Render();
}
