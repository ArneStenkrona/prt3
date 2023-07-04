#ifndef PRT3_WEAPON_GUI_H
#define PRT3_WEAPON_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/weapon.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<Weapon>(
    EditorContext &,
    NodeID
) {}

} // namespace prt3

#endif // PRT3_WEAPON_GUI_H
