#ifndef PRT3_ANIMATED_MODEL_GUI_H
#define PRT3_ANIMATED_MODEL_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/gui_components/component/model_gui_common.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/animated_model.h"
#include "src/engine/rendering/model_manager.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<AnimatedModel>(
    EditorContext & context,
    NodeID id
) {
    show_model_component<AnimatedModel>(context, id);
}

} // namespace prt3

#endif
