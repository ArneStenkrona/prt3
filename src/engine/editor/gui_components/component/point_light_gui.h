#ifndef PRT3_POINT_LIGHT_GUI_H
#define PRT3_POINT_LIGHT_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/point_light.h"

#include "imgui.h"

namespace prt3 {

class ActionSetPointLight : public Action {
public:
    ActionSetPointLight(
        EditorContext & editor_context,
        NodeID node_id,
        PointLight const & light
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_light{light}
    {
        Scene const & scene = editor_context.scene();
        m_original_light =
            scene.get_component<PointLightComponent>(m_node_id).light();
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<PointLightComponent>(m_node_id).light() = m_light;
        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<PointLightComponent>(m_node_id).light() = m_original_light;
        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    PointLight m_light;
    PointLight m_original_light;
};

template<>
void inner_show_component<PointLightComponent>(
    EditorContext & context,
    NodeID id
) {
    Scene & scene = context.scene();
    PointLightComponent & component = scene.get_component<PointLightComponent>(id);

    PointLight light = component.light();
    bool change = false;

    float* color_p = reinterpret_cast<float*>(&light.color);

    ImGui::PushItemWidth(160);

    change |= ImGui::ColorEdit4("albedo", color_p);

    float rescale_factor = 10.0f;
    float step_size = 0.01f;

    float quadratic = rescale_factor * light.quadratic_term - step_size;
    change |= ImGui::DragFloat(
        "quadratic",
        &quadratic,
        step_size,
        0.0f,
        1.0f,
        "%.2f"
    );
    light.quadratic_term = (quadratic + step_size) / rescale_factor;

    float linear = rescale_factor * light.linear_term - step_size;
    change |= ImGui::DragFloat(
        "linear",
        &linear,
        step_size,
        0.0f,
        1.0f,
        "%.2f"
    );
    light.linear_term = (linear + step_size) / rescale_factor;

    float constant = rescale_factor * light.constant_term - step_size;
    change |= ImGui::DragFloat(
        "constant",
        &constant,
        step_size,
        0.0f,
        1.0f,
        "%.2f"
    );
    light.constant_term = (constant + step_size) / rescale_factor;

    ImGui::PopItemWidth();

    if (change) {
        context.editor().perform_action<ActionSetPointLight>(
            id,
            light
        );
    }
}

} // namespace prt3

#endif
