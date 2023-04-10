#ifndef PRT3_NAVIGATION_MESH_GUI_H
#define PRT3_NAVIGATION_MESH_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/component/navigation_mesh.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<NavigationMeshComponent>(
    EditorContext & /*context*/,
    NodeID /*id*/
) {
}

} // namespace prt3

#endif
