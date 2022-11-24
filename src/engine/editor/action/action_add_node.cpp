#include "action_add_node.h"

#include "src/engine/editor/editor_context.h"

#include <cassert>

using namespace prt3;

ActionAddNode::ActionAddNode(
    EditorContext & editor_context,
    NodeID parent_id,
    NodeName const & name
) : m_editor_context{&editor_context},
    m_parent_id{parent_id},
    m_name{name}
{
    Scene & scene = m_editor_context->scene();
    m_node_id = scene.get_next_available_node_id();
}

bool ActionAddNode::apply() {
    Scene & scene = m_editor_context->scene();
    NodeID id = scene.add_node(m_parent_id, m_name);
    assert(id == m_node_id && "id mis-match!");
    m_editor_context->set_selected_node(id);
    return true;
}

bool ActionAddNode::unapply() {
    Scene & scene = m_editor_context->scene();
    scene.remove_node(m_node_id);
    return true;
}
