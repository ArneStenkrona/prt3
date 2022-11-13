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

            auto const & col = sys.get_sphere_collider(tag.id);
            Sphere const & base_shape = col.base_shape();

            float radius = base_shape.radius;
            ImGui::InputFloat(
                "radius",
                &radius,
                0.0f,
                0.0f,
                "%.2f",
                ImGuiInputTextFlags_ReadOnly
            );

            glm::vec3 offset = base_shape.position;
            float* offset_p = reinterpret_cast<float*>(&offset);
            ImGui::InputFloat3(
                "offset",
                offset_p,
                "%.2f",
                ImGuiInputTextFlags_ReadOnly
            );

            break;
        }
        default: {}
    }


    ImGui::PopItemWidth();
}

} // namespace prt3

#endif
