#include "node_inspector.h"

#include "src/util/fixed_string.h"
#include "src/engine/editor/components/panel.h"
#include "src/engine/component/transform.h"

using namespace prt3;

void show_name(NodeName & name) {
    ImGui::InputText("Name", name.data(), name.size());
}

bool show_transform(Transform & transform) {
    bool ret = false;

    begin_group_panel("Transform");

    glm::vec3 & position = transform.position;
    glm::quat & rotation = transform.rotation;
    glm::vec3 & scale = transform.scale;

    float* vecp = reinterpret_cast<float*>(&position);
    ret |= ImGui::InputFloat3("pos", vecp, "%.2f");

    float* rotp = reinterpret_cast<float*>(&rotation);
    ret |= ImGui::InputFloat4("rot", rotp, "%.2f");

    float* scalep = reinterpret_cast<float*>(&scale);
    ret |= ImGui::InputFloat3("scale", scalep, "%.2f");

    end_group_panel();

    return ret;
}

void prt3::node_inspector(EditorContext & context) {
    begin_group_panel("inspector");

    NodeID id = context.get_selected_node();
    Node & node = context.context().scene().get_node(id);
    NodeName & name = context.context().scene().get_node_name(id);

    ImGui::PushID(id);

    ImGui::PushItemWidth(125);
    show_name(name);
    ImGui::PopItemWidth();

    if (show_transform(node.local_transform())) {
        context.invalidate_transform_cache();
    }

    ImGui::PopID();

    end_group_panel();
}
