#ifndef PRT3_SCRIPT_SET_GUI_H
#define PRT3_SCRIPT_SET_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/scene/scene.h"
#include "src/engine/component/script_set.h"

#include "imgui.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace prt3 {

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
        component.remove_script(scene, to_remove);
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

    if (ImGui::Button("Add Script")) {
        ImGui::OpenPopup("select_script_popup");
    }

    ImGui::SameLine();
    if (ImGui::BeginPopup("select_script_popup")) {
        for (size_t i = 0; i < available_uuids.size(); ++i) {
            if (ImGui::Selectable(available_scripts[i])) {
                component.add_script_from_uuid(scene, available_uuids[i]);
            }
        }
        ImGui::EndPopup();
    }

    ImGui::PopItemWidth();
}

} // namespace prt3

#endif
