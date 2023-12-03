#ifndef PRT3_DECAL_GUI_H
#define PRT3_DECAL_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/gui_components/component/component_gui_utility.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/decal.h"

#include "imgui.h"

#include <string>

namespace prt3 {

template<>
void inner_show_component<Decal>(
    EditorContext & context,
    NodeID id
) {
    ImGui::PushItemWidth(125);

    set_texture<Decal>(context, id, offsetof(Decal, m_texture_id));

    edit_field<Decal, glm::vec3>(
        context,
        id,
        "dimension",
        offsetof(Decal, m_dimensions)
    );

    ImGui::PopItemWidth();
}

} // namespace prt3

#endif // PRT3_DECAL_H
