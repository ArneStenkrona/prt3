#ifndef PRT3_SCRIPT_SET_GUI_H
#define PRT3_SCRIPT_SET_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/editor/action/action.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/script_set.h"

#include "imgui.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace prt3 {

class ActionModifyScriptSet : public Action {
public:
    enum class Modification {
        add,
        remove
    };

    ActionModifyScriptSet(
        EditorContext & editor_context,
        NodeID node_id,
        UUID script_uuid,
        Modification modification
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_script_uuid{script_uuid},
        m_modification{modification}
    {}

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        ScriptSet & script_set = scene.get_component<ScriptSet>(m_node_id);

        switch (m_modification) {
            case Modification::add: {
                script_set.add_script_from_uuid(scene, m_script_uuid);
                break;
            }
            case Modification::remove: {
                return script_set.remove_script(scene, m_script_uuid);
                break;
            }
        }
        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        ScriptSet & script_set = scene.get_component<ScriptSet>(m_node_id);

        switch (m_modification) {
            case Modification::add: {
                return script_set.remove_script(scene, m_script_uuid);
                break;
            }
            case Modification::remove: {
                script_set.add_script_from_uuid(scene, m_script_uuid);
                break;
            }
        }
        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    UUID m_script_uuid;
    Modification m_modification;
};

class ActionSetScriptField : public Action {
public:
    ActionSetScriptField(
        EditorContext & editor_context,
        ScriptID script_id,
        Script::FieldValue value,
        size_t field_index
    ) : m_editor_context{&editor_context},
        m_script_id{script_id},
        m_value{value},
        m_field_index{field_index}
    {
        Scene & scene = m_editor_context->scene();
        Script * script = scene.get_script(m_script_id);
        m_original_value = script->get_serialized_field_value(field_index);
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        Script * script = scene.get_script(m_script_id);

        script->set_serialized_field_value(m_field_index, m_value);

        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        Script * script = scene.get_script(m_script_id);

        script->set_serialized_field_value(m_field_index, m_original_value);

        return true;
    }

private:
    EditorContext * m_editor_context;

    ScriptID m_script_id;
    Script::FieldValue m_value;
    Script::FieldValue m_original_value;
    size_t m_field_index;
};

template<>
void inner_show_component<ScriptSet>(
    EditorContext & context,
    NodeID id
) {
    Scene & scene = context.scene();
    ScriptSet & component = scene.get_component<ScriptSet>(id);

    std::vector<ScriptID> const & script_ids = component.get_all_scripts();

    ImGui::PushItemWidth(160);

    ScriptID to_remove = NO_SCRIPT;

    static std::unordered_set<UUID> uuids;
    uuids.clear();

    for (ScriptID const & script_id : script_ids) {
        Script * script = scene.get_script(script_id);

        ImGui::PushID(
            static_cast<int>(script->uuid())
        );

        if (
            begin_group_panel_with_button(
                script->name(),
                "remove"
            )
        ) {
            to_remove = script_id;
        } else {
            uuids.insert(script->uuid());
        }

        auto const & fields = Script::get_serialized_fields(script->uuid());
        void * script_data = reinterpret_cast<void*>(script);

        ImGui::PushItemWidth(160);

        Script::FieldValue val;
        bool changed = false;
        size_t changed_index = 0;
        size_t field_index = 0;
        for (auto & field : fields) {
            switch (field.type) {
                case Script::FieldType::uint8:
                {
                    val.u8 = field.get<uint8_t>(script_data);
                    changed |= ImGui::InputScalar(field.name, ImGuiDataType_U8, &val.u8);
                    break;
                }
                case Script::FieldType::uint16:
                {
                    val.u16 = field.get<uint16_t>(script_data);
                    changed |= ImGui::InputScalar(field.name, ImGuiDataType_U16, &val.u16);
                    break;
                }
                case Script::FieldType::uint32:
                {
                    val.u32 = field.get<uint32_t>(script_data);
                    changed |= ImGui::InputScalar(field.name, ImGuiDataType_U32, &val.u32);
                    break;
                }
                case Script::FieldType::uint64:
                {
                    val.u64 = field.get<uint64_t>(script_data);
                    changed |= ImGui::InputScalar(field.name, ImGuiDataType_U64, &val.u64);
                    break;
                }
                case Script::FieldType::int8:
                {
                    val.i8 = field.get<int8_t>(script_data);
                    changed |= ImGui::InputScalar(field.name, ImGuiDataType_S8, &val.i8);
                    break;
                }
                case Script::FieldType::int16:
                {
                    val.i16 = field.get<int16_t>(script_data);
                    changed |= ImGui::InputScalar(field.name, ImGuiDataType_S16, &val.i16);
                    break;
                }
                case Script::FieldType::int32:
                {
                    val.i32 = field.get<int32_t>(script_data);
                    changed |= ImGui::InputScalar(field.name, ImGuiDataType_S32, &val.i32);
                    break;
                }
                case Script::FieldType::int64:
                {
                    val.i64 = field.get<int64_t>(script_data);
                    changed |= ImGui::InputScalar(field.name, ImGuiDataType_S64, &val.i64);
                    break;
                }
                case Script::FieldType::f32:
                {
                    val.f32 = field.get<float>(script_data);
                    changed |= ImGui::InputFloat(field.name, &val.f32, 0.0f, 0.0f, "%.2f");
                    break;
                }
                case Script::FieldType::f64:
                {
                    val.f64 = field.get<double>(script_data);
                    changed |= ImGui::InputDouble(field.name, &val.f64, 0.0f, 0.0f, "%.2f");
                    break;
                }
                case Script::FieldType::boolean:
                {
                    val.boolean = field.get<bool>(script_data);
                    changed |= ImGui::Checkbox(field.name, &val.boolean);
                    break;
                }
                default: {}
            }

            if (changed) {
                changed_index = field_index;
                break;
            }

            ++field_index;
        }

        ImGui::PopItemWidth();

        if (changed) {
            context.editor().perform_action<ActionSetScriptField>(
                script_id,
                val,
                changed_index
            );
        }

        end_group_panel();

        ImGui::PopID();

    }
    if (to_remove != NO_SCRIPT) {
        context.editor().perform_action<ActionModifyScriptSet>(
            id,
            scene.get_script(to_remove)->uuid(),
            ActionModifyScriptSet::Modification::remove
        );
    }

    static std::vector<UUID> available_uuids;
    available_uuids.resize(0);
    static std::vector<const char *> available_scripts;
    available_scripts.resize(0);

    for (auto const & pair : Script::script_names()) {
        if (uuids.find(pair.first) == uuids.end()) {
            available_uuids.push_back(pair.first);
            available_scripts.push_back(pair.second);
        }
    }

    if (ImGui::Button("add script")) {
        ImGui::OpenPopup("select_script_popup");
    }

    ImGui::SameLine();
    if (ImGui::BeginPopup("select_script_popup")) {
        for (size_t i = 0; i < available_uuids.size(); ++i) {
            if (ImGui::Selectable(available_scripts[i])) {
                context.editor().perform_action<ActionModifyScriptSet>(
                    id,
                    available_uuids[i],
                    ActionModifyScriptSet::Modification::add
                );
            }
        }
        ImGui::EndPopup();
    }

    ImGui::PopItemWidth();
}

} // namespace prt3

#endif
