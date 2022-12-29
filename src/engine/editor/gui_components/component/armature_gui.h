#ifndef PRT3_ARMATURE_GUI_H
#define PRT3_ARMATURE_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/gui_components/component/model_gui_common.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/component/armature.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<Armature>(
    EditorContext & context,
    NodeID id
) {
    show_model_component<Armature>(context, id, true);
}

} // namespace prt3

#endif
