#include "node_inspector.h"

#include "src/util/fixed_string.h"
#include "src/engine/component/component.h"
#include "src/engine/editor/gui_components/panel.h"
#include "src/engine/component/transform.h"
#include "src/util/template_util.h"
#include "src/engine/editor/gui_components/component/collider_gui.h"
#include "src/engine/editor/gui_components/component/material_gui.h"
#include "src/engine/editor/gui_components/component/mesh_gui.h"
#include "src/engine/editor/gui_components/component/model_gui.h"
#include "src/engine/editor/gui_components/component/point_light_gui.h"
#include "src/engine/editor/gui_components/component/script_set_gui.h"

using namespace prt3;

template<typename T>
void show_component(EditorContext & context, NodeID id) {
    Scene & scene = context.context().edit_scene();

    if (scene.has_component<T>(id)) {
        ImGui::PushID(T::name());

        bool remove = begin_group_panel_with_button(
            T::name(),
            "remove"
        );

        inner_show_component<T>(context, id);

        end_group_panel();

        if (remove) {
            scene.remove_component<T>(id);
        }

        ImGui::PopID();
    }
}

void do_in_order(EditorContext &) {}

template<class...Fs>
void do_in_order(EditorContext & context, NodeID id, Fs&&...fs) {
    using discard=int[];
    (void)discard{0, (void(
    std::forward<Fs>(fs)(context, id)
    ),0)... };
}

template<class...Ts>
void show_components(EditorContext & context, NodeID id, type_pack<Ts...>) {
    do_in_order(context, id, show_component<Ts>... );
}

template<typename T>
void inner_show_add_component(EditorContext & context, NodeID id) {
    Scene & scene = context.context().edit_scene();

    if (!scene.has_component<T>(id)) {
        if (ImGui::Selectable(T::name())) {
            scene.add_component<T>(id);
        }
    }
}

template<class...Ts>
void show_add_component(EditorContext & context, NodeID id, type_pack<Ts...>) {
    if (ImGui::Button("add component")) {
        ImGui::OpenPopup("select_component_popup");
    }

    ImGui::SameLine();
    if (ImGui::BeginPopup("select_component_popup")) {
        do_in_order(context, id, inner_show_add_component<Ts>... );
        ImGui::EndPopup();
    }
}

void show_name(NodeName & name) {
    ImGui::InputText("name", name.data(), name.buf_size());
}

bool show_transform(Transform & transform) {
    bool ret = false;

    begin_group_panel("transform");
    ImGui::PushItemWidth(160);

    glm::vec3 & position = transform.position;
    glm::quat & rotation = transform.rotation;
    glm::vec3 & scale = transform.scale;

    float* vecp = reinterpret_cast<float*>(&position);
    ret |= ImGui::InputFloat3("position", vecp, "%.2f");

    float* rotp = reinterpret_cast<float*>(&rotation);
    ret |= ImGui::InputFloat4("rotation", rotp, "%.2f");

    float* scalep = reinterpret_cast<float*>(&scale);
    ret |= ImGui::InputFloat3("scale", scalep, "%.2f");

    ImGui::PopItemWidth();

    end_group_panel();

    return ret;
}

void prt3::node_inspector(EditorContext & context) {
    NodeID id = context.get_selected_node();
    Node & node = context.context().edit_scene().get_node(id);
    NodeName & name = context.context().edit_scene().get_node_name(id);

    ImGui::PushID(id);

    ImGui::PushItemWidth(125);
    show_name(name);
    ImGui::PopItemWidth();

    if (show_transform(node.local_transform())) {
        context.invalidate_transform_cache();
    }

    show_components(context, id, ComponentTypes{});
    show_add_component(context, id, ComponentTypes{});

    ImGui::PopID();
}
