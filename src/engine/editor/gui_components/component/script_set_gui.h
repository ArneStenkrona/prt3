#ifndef PRT3_SCRIPT_SET_GUI_H
#define PRT3_SCRIPT_SET_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/script_set.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<ScriptSet>(
    EditorContext & context,
    NodeID id
) {
//     Scene & scene = context.scene();
//     ModelManager & man = context.get_model_manager();
//     ScriptSet & component = scene.get_component<ScriptSet>(id);
}

} // namespace prt3

#endif
