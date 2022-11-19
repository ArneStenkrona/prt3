#include "node_transform_action.h"

#include "src/engine/editor/editor_context.h"

using namespace prt3;

NodeTransformAction::NodeTransformAction(
    EditorContext & editor_context,
    NodeID node_id,
    Transform const & original_transform,
    Transform const & new_transform
) : m_editor_context{&editor_context},
    m_node_id{node_id},
    m_original_transform{original_transform},
    m_new_transform{new_transform} {
}

bool NodeTransformAction::apply() {
    Scene & scene = m_editor_context->scene();
    Node & node = scene.get_node(m_node_id);
    node.local_transform() = m_new_transform;
    return true;
}

bool NodeTransformAction::unapply() {
    Scene & scene = m_editor_context->scene();
    Node & node = scene.get_node(m_node_id);
    node.local_transform() = m_original_transform;
    return true;
}
