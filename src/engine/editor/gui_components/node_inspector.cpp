#include "node_inspector.h"

#include "src/engine/editor/editor.h"
#include "src/util/fixed_string.h"
#include "src/engine/component/component.h"
#include "src/engine/editor/gui_components/panel.h"
#include "src/engine/component/transform.h"
#include "src/util/template_util.h"
#include "src/util/file_util.h"
#include "src/engine/editor/gui_components/component/armature_gui.h"
#include "src/engine/editor/gui_components/component/animated_mesh_gui.h"
#include "src/engine/editor/gui_components/component/animated_model_gui.h"
#include "src/engine/editor/gui_components/component/collider_gui.h"
#include "src/engine/editor/gui_components/component/door_gui.h"
#include "src/engine/editor/gui_components/component/material_gui.h"
#include "src/engine/editor/gui_components/component/mesh_gui.h"
#include "src/engine/editor/gui_components/component/model_gui.h"
#include "src/engine/editor/gui_components/component/point_light_gui.h"
#include "src/engine/editor/gui_components/component/script_set_gui.h"
#include "src/engine/editor/action/action_transform_node.h"
#include "src/engine/editor/action/action_add_component.h"
#include "src/engine/editor/action/action_remove_component.h"
#include "src/engine/scene/prefab.h"

#include <fstream>

using namespace prt3;

class ActionSetNodeName : public Action {
public:
    ActionSetNodeName(
        EditorContext & editor_context,
        NodeID node_id,
        NodeName const & name
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_name{name}
    {
        Scene const & scene = editor_context.scene();
        m_original_name = scene.get_node_name(m_node_id);
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        scene.get_node_name(m_node_id) = m_name;
        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        scene.get_node_name(m_node_id) = m_original_name;
        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    NodeName m_name;
    NodeName m_original_name;
};

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
            context.editor().perform_action<ActionRemoveComponent<T> >(id);
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
            context.editor().perform_action<ActionAddComponent<T> >(id);
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

bool show_name(NodeName & name) {
    return ImGui::InputText(
        "name",
        name.data(),
        name.buf_size(),
        ImGuiInputTextFlags_NoUndoRedo
    );
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

void show_save_as_prefab(EditorContext & context, NodeID id) {
    thread_local bool open = false;

    if (ImGui::Button("save prefab")) {
        open = true;
    }

    if (open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("save prefab");

        if (file_dialog.showFileDialog("save prefab",
            imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(0, 0)
            )
        ) {
            std::string const & path = file_dialog.selected_path;

            std::ofstream out(path, std::ios::binary);
            Prefab::serialize_node(context.scene(), id, out);
            out.close();
            emscripten_save_file_via_put(path);
        }
        open = ImGui::IsPopupOpen(ImGui::GetID("save prefab"), 0);
    }
}

void prt3::node_inspector(EditorContext & context) {
    NodeID id = context.get_selected_node();
    Node & node = context.context().edit_scene().get_node(id);
    NodeName name = context.context().edit_scene().get_node_name(id);

    ImGui::PushID(id);

    ImGui::PushItemWidth(125);
    if (show_name(name)) {
        context.editor().perform_action<ActionSetNodeName>(
            id,
            name
        );
    }

    ImGui::PopItemWidth();

    show_save_as_prefab(context, id);

    Transform transform = node.local_transform();
    if (show_transform(transform)) {
        context.editor().perform_action<ActionTransformNode>(
            id,
            node.local_transform(),
            transform
        );
    }

    show_components(context, id, ComponentTypes{});
    show_add_component(context, id, ComponentTypes{});

    ImGui::PopID();
}
