#ifndef PRT3_COLLIDER_GUI_H
#define PRT3_COLLIDER_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/collider_component.h"
#include "src/engine/editor/action/action.h"
#include "src/util/mem.h"

#include "imgui.h"

#include <sstream>
#include <iostream>

namespace prt3 {

template<typename ConstructorArgType>
class ActionSetCollider : public Action {
public:
    ActionSetCollider(
        EditorContext & editor_context,
        NodeID node_id,
        ConstructorArgType const & constructor_arg
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_constructor_arg{constructor_arg}
    {
        Scene const & scene = m_editor_context->scene();

        std::stringstream stream;
        scene.get_component<ColliderComponent>(m_node_id)
            .serialize(stream, scene);

        std::string const & s = stream.str();
        m_original_data.reserve(s.size());
        m_original_data.assign(s.begin(), s.end());
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<ColliderComponent>(m_node_id)
            .set_collider(scene, m_constructor_arg);
        return true;
    }

    virtual bool unapply() {
        imemstream in(m_original_data.data(), m_original_data.size());
        Scene & scene = m_editor_context->scene();
        scene.get_component<ColliderComponent>(m_node_id).deserialize(in, scene);
        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    std::vector<char> m_original_data;

    ConstructorArgType m_constructor_arg;
};

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

    static char const * types[] = { "mesh", "sphere", "box" };
    static ColliderType types_enum[] = {
        ColliderType::collider_type_mesh,
        ColliderType::collider_type_sphere,
        ColliderType::collider_type_box,
    };

    static int enum_to_index[] = {
        -1, // ColliderType::collider_type_none
         0, // ColliderType::collider_type_mesh,
         1, // ColliderType::collider_type_sphere,
         2  // ColliderType::collider_type_box
    };

    static int current_type = 0;
    current_type = enum_to_index[tag.type];

    ImGui::Combo("type", &current_type, types, IM_ARRAYSIZE(types));

    if (types_enum[current_type] != tag.type) {
        tag.type = types_enum[current_type];

        switch (tag.type) {
            case ColliderType::collider_type_mesh: {
                context.editor().perform_action<ActionSetCollider<
                        std::vector<glm::vec3>
                    > >(
                    id,
                    std::vector<glm::vec3>{}
                );
                tag = component.tag();
                break;
            }
            case ColliderType::collider_type_sphere: {
                Sphere sphere{};
                sphere.radius = 1.0f;
                context.editor().perform_action<ActionSetCollider<
                    Sphere
                > >(
                    id,
                    sphere
                );
                tag = component.tag();
                break;
            }
            case ColliderType::collider_type_box: {
                Box box{};
                box.dimensions = {1.0f, 1.0f, 1.0f};
                context.editor().perform_action<ActionSetCollider<
                    Box
                > >(
                    id,
                    box
                );
                tag = component.tag();
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
                        context.editor().perform_action<ActionSetCollider<
                            Model
                        > >(
                            id,
                            model
                        );
                        tag = component.tag();
                    }
                }
                ImGui::EndPopup();
            }
            break;
        }
        case ColliderType::collider_type_sphere: {
            auto & col = sys.get_sphere_collider(tag.id);
            Sphere base_shape = col.base_shape();

            bool edited = false;

            edited |= ImGui::DragFloat(
                "radius",
                &base_shape.radius,
                0.01f,
                0.0f,
                std::numeric_limits<float>::max(),
                "%.2f"
            );

            float* offset_p = reinterpret_cast<float*>(&base_shape.position);
            edited |= ImGui::InputFloat3(
                "offset",
                offset_p,
                "%.2f"
            );

            if (edited) {
                context.editor().perform_action<ActionSetCollider<
                    Sphere
                > >(
                    id,
                    base_shape
                );
                tag = component.tag();
            }

            break;
        }
        case ColliderType::collider_type_box: {
            auto & col = sys.get_box_collider(tag.id);

            bool edited = false;

            glm::vec3 dimensions = col.dimensions();
            float* dim_p = reinterpret_cast<float*>(&dimensions);
            edited |= ImGui::InputFloat3(
                "dimensions",
                dim_p,
                "%.2f"
            );

            glm::vec3 center = col.center();
            float* center_p = reinterpret_cast<float*>(&center);
            edited |= ImGui::InputFloat3(
                "center",
                center_p,
                "%.2f"
            );

            if (edited) {
                context.editor().perform_action<ActionSetCollider<
                    Box
                > >(
                    id,
                    Box{dimensions, center}
                );
                tag = component.tag();
            }

            break;
        }
        default: {}
    }


    ImGui::PopItemWidth();
}

} // namespace prt3

#endif
