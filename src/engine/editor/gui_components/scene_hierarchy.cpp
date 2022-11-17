#include "scene_hierarchy.h"

#include "src/util/fixed_string.h"
#include "src/engine/editor/gui_components/panel.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include <vector>
#include <cstring>
#include <algorithm>

using namespace prt3;

void prt3::scene_hierarchy(EditorContext & context) {
    Scene & scene = context.context().edit_scene();
    NodeID root_id = scene.get_root_id();
    std::vector<Node> & nodes = context.get_scene_nodes();

    static std::vector<NodeID> node_ids;
    node_ids.resize(nodes.size());
    static std::vector<bool> expanded = { true };
    expanded.resize(nodes.size());

    static constexpr size_t max_depth = 32;
    static constexpr size_t N = NodeName::Size + 2 * (max_depth + 1);
    static std::vector<FixedString<N> > display_names;
    display_names.resize(nodes.size());

    struct QueueElement {
        NodeID id;
        unsigned int depth;
    };

    static std::vector<QueueElement> queue;
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
    static int32_t selected = 0;
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

    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 40);


    static std::vector<char*> name_data;
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

        NodeID id = scene.add_node(parent, "new node");

        context.set_selected_node(id);
    }

    if (context.get_selected_node() != NO_NODE &&
        context.get_selected_node() != context.scene().get_root_id()) {
        ImGui::SameLine();
        if (ImGui::Button("remove node")) {
            Scene & scene = context.scene();
            NodeID to_remove = context.get_selected_node();

            NodeID parent = context.scene().get_node(to_remove).parent_id();
            context.set_selected_node(parent);

            expanded[to_remove] = false;

            scene.remove_node(to_remove);
        }
    }
}
