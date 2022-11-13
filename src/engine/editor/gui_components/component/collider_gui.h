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

    char const * types[] = { "mesh", "sphere" };
    ColliderType types_enum[] = {
        ColliderType::collider_type_mesh,
        ColliderType::collider_type_sphere
    };
    static int current_type = 0;
    current_type = tag.type;

    ImGui::Combo("type", &current_type, types, IM_ARRAYSIZE(types));

    if (types_enum[current_type] != tag.type) {
        tag.type = types_enum[current_type];

        switch (tag.type) {
            case ColliderType::collider_type_mesh: {
                component.set_mesh_collider(
                    scene,
                    std::vector<glm::vec3>{}
                );
                break;
            }
            case ColliderType::collider_type_sphere: {
                Sphere sphere{};
                sphere.radius = 1.0f;
                component.set_sphere_collider(
                    scene,
                    sphere
                );
                break;
            }
            default: {}
        }
    }

    switch (tag.type) {
        case ColliderType::collider_type_mesh: {
            if (ImGui::Button("load data from model")) {
                ImGui::OpenPopup("select_model_popup");
            }

            ModelManager & man = context.get_model_manager();
            std::vector<Model> const & models = man.models();

            ImGui::SameLine();
            if (ImGui::BeginPopup("select_model_popup")) {
                for (Model const & model : models) {
                    if (ImGui::Selectable(model.path().c_str())) {
                        component.set_mesh_collider(scene, model);
                    }
                }
                ImGui::EndPopup();
            }
            break;
        }
        case ColliderType::collider_type_sphere: {
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
