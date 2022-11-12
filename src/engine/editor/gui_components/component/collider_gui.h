#ifndef PRT3_COLLIDER_GUI_H
#define PRT3_COLLIDER_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/collider_component.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<ColliderComponent>(
    EditorContext & context,
    NodeID id
) {
//     Scene & scene = context.scene();
//     ModelManager & man = context.get_model_manager();
//     ColliderComponent & component = scene.get_component<ColliderComponent>(id);
}

} // namespace prt3

#endif
