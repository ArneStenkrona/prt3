#ifndef PRT3_MODEL_GUI_H
#define PRT3_MODEL_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/gui_components/component/model_gui_common.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/component/model.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<ModelComponent>(
    EditorContext & context,
    NodeID id
) {
    show_model_component<ModelComponent>(context, id, false);
    show_unpack_model_component<ModelComponent>(context, id, false);
}

} // namespace prt3

#endif
