#ifndef PRT3_COMPONENT_GUI_UTILITY_H
#define PRT3_COMPONENT_GUI_UTILITY_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/util/fixed_string.h"

#include "imgui.h"

#include <cstdint>

namespace prt3 {

template<typename ComponentType, typename FieldType>
class ActionSetField : public Action {
public:
    ActionSetField(
        EditorContext & editor_context,
        NodeID node_id,
        size_t offset,
        FieldType const & value
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_offset{offset},
        m_value{value}
    {
        ComponentType & component =
            editor_context
                .scene().get_component<ComponentType>(node_id);

        m_original_value = *reinterpret_cast<FieldType*>(
            reinterpret_cast<char*>(&component) + m_offset
        );
    }

protected:
    virtual bool apply() {
        ComponentType & component =
            m_editor_context
                ->scene().get_component<ComponentType>(m_node_id);

        *reinterpret_cast<FieldType*>(
            reinterpret_cast<char*>(&component) + m_offset
        ) = m_value;
        return true;
    }

    virtual bool unapply() {
        ComponentType & component =
            m_editor_context
                ->scene().get_component<ComponentType>(m_node_id);

        *reinterpret_cast<FieldType*>(
            reinterpret_cast<char*>(&component) + m_offset
        ) = m_original_value;
        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    size_t m_offset;

    FieldType m_value;
    FieldType m_original_value;
};

template<typename T>
bool display_value(char const * label, T & val);

template<>
bool display_value<int32_t>(char const * label, int32_t & val) {
    return ImGui::InputScalar(
        label,
        ImGuiDataType_S32,
        &val
    );
}

template<typename ComponentType, typename FieldType>
void edit_field(
    EditorContext & context,
    NodeID id,
    char const * label,
    size_t offset
) {
    ComponentType & component =
        context.scene().get_component<ComponentType>(id);

    FieldType val = *reinterpret_cast<FieldType*>(
        reinterpret_cast<char*>(&component) + offset
    );
    if (display_value(label, val)) {
        context.editor().perform_action<ActionSetField<
            ComponentType, FieldType>
        >(
            id,
            offset,
            val
        );
    }
}

template<typename ComponentType, size_t N>
void edit_fixed_string_path_field(
    EditorContext & context,
    NodeID id,
    char const * label,
    size_t offset
) {
    ImGui::PushID(label);

    thread_local bool open = false;
    if (ImGui::SmallButton("set")) {
        open = true;
    }
    ImGui::SameLine();

    ComponentType & component =
        context.scene().get_component<ComponentType>(id);

    FixedString<N> str =
        *reinterpret_cast<FixedString<N>*>(
            reinterpret_cast<char*>(&component) + offset
        );

    ImGui::InputText(
        label,
        str.data(),
        str.writeable_size(),
        ImGuiInputTextFlags_ReadOnly
    );

    if (open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("set path");

        if (file_dialog.showFileDialog("set path",
            imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(0, 0))
        ) {
            FixedString<N> val = file_dialog.selected_path.c_str();

            context.editor().perform_action<ActionSetField<
                ComponentType, FixedString<N>>
            >(
                id,
                offset + (size_t(str.data()) - size_t(&str)),
                val
            );
        }
        open = ImGui::IsPopupOpen(ImGui::GetID("set path"), 0);
    }

    ImGui::PopID();
}

} // namespace prt3

#endif
