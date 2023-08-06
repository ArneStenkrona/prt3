#ifndef PRT3_DOOR_GUI_H
#define PRT3_DOOR_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/gui_components/component/component_gui_utility.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/door.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<Door>(
    EditorContext & context,
    NodeID id
) {
    ImGui::PushItemWidth(125);

    edit_fixed_string_path_field<Door, DoorPathType::Size>(
        context,
        id,
        "scene",
        offsetof(Door, m_destination_scene_path)
    );

    edit_field<Door, DoorID>(
        context,
        id,
        "door id",
        offsetof(Door, m_id)
    );

    edit_field<Door, DoorID>(
        context,
        id,
        "destination door id",
        offsetof(Door, m_destination_id)
    );

    edit_field<Door, glm::vec3>(
        context,
        id,
        "entry offset",
        offsetof(Door, m_entry_offset)
    );

    ImGui::PopItemWidth();
}

} // namespace prt3

#endif // PRT3_DOOR_GUI_H
