#ifndef PRT3_POINT_LIGHT_GUI_H
#define PRT3_POINT_LIGHT_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/point_light.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<PointLightComponent>(
    EditorContext & context,
    NodeID id
) {
//     Scene & scene = context.scene();
//     ModelManager & man = context.get_model_manager();
//     PointLightComponent & component = scene.get_component<PointLightComponent>(id);
}

} // namespace prt3

#endif
