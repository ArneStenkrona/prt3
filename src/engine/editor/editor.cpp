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
   m_gizmo_manager{m_editor_context, m_camera.get_camera()} {
}

void Editor::update(float delta_time) {
    ImGui::NewFrame();

    editor_gui(m_editor_context);
    bool gizmo_interaction = m_gizmo_manager.update();

    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) &&
        !gizmo_interaction) {
        m_camera.update(delta_time, m_context.input());

        if (m_context.input().get_key_down(KEY_CODE_MOUSE_RIGHT)) {
            int x, y;
            m_context.input().get_cursor_position(x, y);

            NodeID id = m_context.renderer().get_selected(x, y);
            m_editor_context.set_selected_node(id);
        }
    }

    ImGui::Render();
}
