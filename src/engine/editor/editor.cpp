#include "editor.h"

#include "src/engine/editor/editor_context.h"
#include "src/engine/editor/components/editor_gui.h"

#include "imgui.h"

using namespace prt3;

Editor::Editor(Context & context)
 : m_context{context},
   m_camera{context.renderer().window_width(),
            context.renderer().window_height()},
   m_editor_context{*this, m_context} {
}

void Editor::update(float delta_time) {
    m_camera.update(delta_time, m_context.input());
    ImGui::NewFrame();
    editor_gui(m_editor_context);
    ImGui::Render();
}
