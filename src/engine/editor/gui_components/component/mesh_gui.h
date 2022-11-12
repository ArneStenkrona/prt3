#ifndef PRT3_MESH_GUI_H
#define PRT3_MESH_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/mesh.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<Mesh>(
    EditorContext & context,
    NodeID id
) {
//     Scene & scene = context.scene();
//     ModelManager & man = context.get_model_manager();
//     Mesh & component = scene.get_component<Mesh>(id);
}

} // namespace prt3

#endif
