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
    Scene & scene = context.scene();
    ColliderComponent & component = scene.get_component<ColliderComponent>(id);

    PhysicsSystem & sys = scene.physics_system();

    ColliderTag tag = component.tag();

    ImGui::PushItemWidth(160);

    const char* types[] = { "mesh", "sphere" };
    static int current_type = 0;
    ImGui::Combo("type", &current_type, types, IM_ARRAYSIZE(types));

    switch (tag.type) {
        case ColliderType::collider_type_mesh: {
            current_type = 0;
            break;
        }
        case ColliderType::collider_type_sphere: {
            current_type = 1;

            auto & col = sys.get_sphere_collider(tag.id);
            Sphere & base_shape = col.base_shape();

            ImGui::DragFloat(
                "radius",
                &base_shape.radius,
                0.01f,
                0.0f,
                std::numeric_limits<float>::max(),
                "%.2f"
            );

            float* offset_p = reinterpret_cast<float*>(&base_shape.position);
            ImGui::InputFloat3(
                "offset",
                offset_p,
                "%.2f"
            );

            break;
        }
        default: {}
    }


    ImGui::PopItemWidth();
}

} // namespace prt3

#endif
