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
