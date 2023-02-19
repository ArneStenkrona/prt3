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

template<>
bool display_value<glm::vec3>(char const * label, glm::vec3 & val) {
    float* vecp = reinterpret_cast<float*>(&val);
    return ImGui::InputFloat3(
        label,
        vecp,
        "%.2f"
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

// Thanks, JSandusky!
// https://gist.github.com/JSandusky/af0e94011aee31f7b05ed2257d347637
template<typename IntType>
bool bit_field(
    const char* label,
    IntType * bits,
    unsigned* hoverIndex = nullptr
) {
    unsigned n_bits = std::numeric_limits<IntType>::digits;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    unsigned oldFlags = window->Flags;
    const ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, 0x0, true);
    const ImVec2 smallLabelSize = ImVec2(label_size.x * 0.5f, label_size.y * 0.5f);

    const float spacingUnit = 2.0f;

    bool anyPressed = false;
    ImVec2 currentPos = window->DC.CursorPos;
    for (unsigned i = 0; i < n_bits; ++i)
    {
        const void* lbl = (void*)(label + i);
        const ImGuiID localId = window->GetID(lbl);
        if (i == 16)
        {
            currentPos.x = window->DC.CursorPos.x;
            currentPos.y += smallLabelSize.y + style.FramePadding.y * 2 + spacingUnit /*little bit of space*/;
        }
        if (i == 8 || i == 24)
            currentPos.x += smallLabelSize.y;

        const ImRect check_bb(currentPos, { currentPos.x + smallLabelSize.y + style.FramePadding.y * 2, currentPos.y + smallLabelSize.y + style.FramePadding.y * 2 });

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(check_bb, localId, &hovered, &held, ImGuiButtonFlags_PressedOnClick);
        if (pressed)
            *bits ^= (1 << i);

        if (hovered && hoverIndex)
            *hoverIndex = i;

        ImGui::RenderFrame(check_bb.Min, check_bb.Max, ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg));
        if (*bits & (1 << i))
        {
            const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
            const float pad = ImMax(spacingUnit, (float)(int)(check_sz / 4.0f));
            window->DrawList->AddRectFilled(
            { check_bb.Min.x + pad, check_bb.Min.y + pad },
            { check_bb.Max.x - pad, check_bb.Max.y - pad }, ImGui::GetColorU32(ImGuiCol_CheckMark));
        }

        anyPressed |= pressed;
        currentPos.x = check_bb.Max.x + spacingUnit;
    }

    const ImRect matrix_bb(window->DC.CursorPos,
        { window->DC.CursorPos.x + (smallLabelSize.y + style.FramePadding.y * 2) * 16 /*# of checks in a row*/ + smallLabelSize.y /*space between sets of 8*/ + 15 * spacingUnit /*spacing between each check*/,
            window->DC.CursorPos.y + ((smallLabelSize.y + style.FramePadding.y * 2) * 2 /*# of rows*/ + spacingUnit /*spacing between rows*/) });

    ImGui::ItemSize(matrix_bb, style.FramePadding.y);

    ImRect total_bb = matrix_bb;

    if (label_size.x > 0)
        ImGui::SameLine(0, style.ItemInnerSpacing.x);

    const ImRect text_bb({ window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y }, { window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y });
    if (label_size.x > 0)
    {
        ImGui::ItemSize(ImVec2(text_bb.GetWidth(), matrix_bb.GetHeight()), style.FramePadding.y);
        total_bb = ImRect(ImMin(matrix_bb.Min, text_bb.Min), ImMax(matrix_bb.Max, text_bb.Max));
    }

    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    if (label_size.x > 0.0f)
        ImGui::RenderText(text_bb.GetTL(), label);

    window->Flags = oldFlags;
    return anyPressed;
}

} // namespace prt3

#endif
