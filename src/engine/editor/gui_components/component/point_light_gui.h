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
    Scene & scene = context.scene();
    PointLightComponent & component = scene.get_component<PointLightComponent>(id);

    PointLight & light = component.light();

    float* color_p = reinterpret_cast<float*>(&light.color);

    ImGui::PushItemWidth(160);

    ImGui::ColorEdit4("albedo", color_p);

    ImGui::DragFloat(
        "quadratic",
        &light.quadratic_term,
        0.01f,
        0.01f,
        10.0f,
        "%.2f"
    );

    ImGui::DragFloat(
        "linear",
        &light.linear_term,
        0.01f,
        0.01f,
        10.0f,
        "%.2f"
    );

    ImGui::DragFloat(
        "constant",
        &light.constant_term,
        0.01f,
        0.01f,
        10.0f,
        "%.2f"
    );

    ImGui::PopItemWidth();
}

} // namespace prt3

#endif
