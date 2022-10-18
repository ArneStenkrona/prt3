#include "editor.h"

#include "imgui.h"

using namespace prt3;

Editor::Editor(Context & context)
 : m_context{context},
   m_camera{context.renderer().window_width(),
            context.renderer().window_height()} {

}

void Editor::update(float delta_time) {
    m_camera.update(delta_time, m_context.input());
}
