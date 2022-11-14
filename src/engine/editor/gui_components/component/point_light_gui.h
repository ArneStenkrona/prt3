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

    float rescale_factor = 10.0f;
    float step_size = 0.01f;

    float quadratic = rescale_factor * light.quadratic_term - step_size;
    ImGui::DragFloat(
        "quadratic",
        &quadratic,
        step_size,
        0.0f,
        1.0f,
        "%.2f"
    );
    light.quadratic_term = (quadratic + step_size) / rescale_factor;

    float linear = rescale_factor * light.linear_term - step_size;
    ImGui::DragFloat(
        "linear",
        &linear,
        step_size,
        0.0f,
        1.0f,
        "%.2f"
    );
    light.linear_term = (linear + step_size) / rescale_factor;

    float constant = rescale_factor * light.constant_term - step_size;
    ImGui::DragFloat(
        "constant",
        &constant,
        step_size,
        0.0f,
        1.0f,
        "%.2f"
    );
    light.constant_term = (constant + step_size) / rescale_factor;

    ImGui::PopItemWidth();
}

} // namespace prt3

#endif
