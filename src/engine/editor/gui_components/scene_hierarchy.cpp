#include "scene_hierarchy.h"

#include "src/engine/editor/editor.h"
#include "src/engine/editor/action/action_add_node.h"
#include "src/engine/editor/action/action_remove_node.h"
#include "src/engine/editor/action/action_instantiate_prefab.h"
#include "src/util/fixed_string.h"
#include "src/engine/editor/gui_components/panel.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include <vector>
#include <cstring>
#include <algorithm>
#include <fstream>

using namespace prt3;

void prt3::scene_hierarchy(EditorContext & context) {
    Scene & scene = context.context().edit_scene();
    NodeID root_id = scene.get_root_id();
    std::vector<Node> & nodes = context.get_scene_nodes();

    thread_local std::vector<NodeID> node_ids;
    node_ids.resize(nodes.size());
    thread_local std::vector<bool> expanded = { true };
    expanded.resize(nodes.size());

    thread_local constexpr size_t max_depth = 32;
    thread_local constexpr size_t N = NodeName::Size + 2 * (max_depth + 1);
    thread_local std::vector<FixedString<N> > display_names;
    display_names.resize(nodes.size());

    struct QueueElement {
        NodeID id;
        unsigned int depth;
    };

    thread_local std::vector<QueueElement> queue;
    if (root_id != NO_NODE) {
        queue.push_back({root_id, 0});
    }

    NodeID select_id = context.get_selected_node();
    if (select_id != NO_NODE) {
        NodeID curr = scene.get_node(select_id).parent_id();
        while (curr != NO_NODE) {
            Node const & node = scene.get_node(curr);
            expanded[curr] = true;

            curr = node.parent_id();
        }
    }

    unsigned int n_displayed = 0;
    thread_local int32_t selected = 0;
    selected = 0;

    while (!queue.empty()) {
        NodeID id = queue.back().id;
        unsigned int depth = queue.back().depth;
        queue.pop_back();
        Node & node = nodes[id];

        node_ids[n_displayed] = id;

        char * begin = display_names[n_displayed].data();

        size_t indent_len =  2 * std::min(size_t(depth), max_depth);
        for (size_t i = 0; i < indent_len; ++i) {
            *begin = ' ';
            ++begin;
        }
        if (node.children_ids().empty()) {
            *begin = ' ';
            ++begin;
            *begin = ' ';
            ++begin;
        } else {
            *begin = expanded[id] ? 'v' : '>';
            ++begin;
            *begin = ' ';
            ++begin;
        }
        strncpy(
            begin,
            context.context().edit_scene().get_node_name(id).data(),
            N - (indent_len + 2) - 1 // -1 is to ensure null-termination
        );

        if (expanded[id]) {
            for (NodeID child_id : node.children_ids()) {
                queue.push_back({child_id, depth + 1});
            }
        }

        ++n_displayed;
    }

    for (size_t i = 0; i < node_ids.size(); ++i) {
        if (node_ids[i] == context.get_selected_node()) {
            selected = i;
        }
    }

    begin_group_panel("nodes");

    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 10);


    thread_local std::vector<char*> name_data;
    name_data.resize(n_displayed);
    for (unsigned int i = 0; i < name_data.size(); ++i) {
        name_data[i] = display_names[i].data();
    }

    ImGui::ListBox(
        "##nodes",
        &selected,
        name_data.data(),
        name_data.size(),
        20
    );

    NodeID selected_id = node_ids[selected];
    context.set_selected_node(node_ids[selected]);
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        expanded[selected_id] = !expanded[selected_id];
    }

    end_group_panel();

    if (ImGui::Button("add node")) {
        Scene & scene = context.scene();

        NodeID parent = context.get_selected_node();
        if (parent == NO_NODE) {
            parent = scene.get_root_id();
        }

        context.editor().perform_action<ActionAddNode>(
            parent,
            NodeName{"new node"}
        );
    }

    if (context.get_selected_node() != NO_NODE &&
        context.get_selected_node() != context.scene().get_root_id()) {
        ImGui::SameLine();
        if (ImGui::Button("remove node")) {
            NodeID to_remove = context.get_selected_node();

            NodeID parent = context.scene().get_node(to_remove).parent_id();
            context.set_selected_node(parent);

            expanded[to_remove] = false;

            context.editor().perform_action<ActionRemoveNode>(
                to_remove
            );
        }
    }

    thread_local bool load_open = false;
    if (ImGui::Button("load prefab")) {
        load_open = true;
    }
    if (load_open) {
        auto & file_dialog = context.file_dialog();
        ImGui::OpenPopup("load prefab");

        if (file_dialog.showFileDialog("load prefab",
            imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(0, 0))
        ) {
            std::string const & path = file_dialog.selected_path;

            NodeID parent = context.get_selected_node();
            if (parent == NO_NODE) {
                parent = scene.get_root_id();
            }

            context.editor().perform_action<ActionInstantiatePrefab>(
                parent,
                path.c_str()
            );
        }
        load_open = ImGui::IsPopupOpen(ImGui::GetID("load prefab"), 0);
    }
}
