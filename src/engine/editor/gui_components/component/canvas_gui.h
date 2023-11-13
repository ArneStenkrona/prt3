#ifndef PRT3_CANVAS_GUI_H
#define PRT3_CANVAS_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/component/canvas/canvas.h"

namespace prt3 {

template<>
void inner_show_component<Canvas>(
    EditorContext &,
    NodeID
) {}

} // namespace prt3

#endif
