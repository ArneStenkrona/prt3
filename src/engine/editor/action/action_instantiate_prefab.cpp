#include "action_instantiate_prefab.h"

#include "src/engine/editor/editor_context.h"

using namespace prt3;

ActionInstantiatePrefab::ActionInstantiatePrefab(
    EditorContext & editor_context,
    NodeID parent_id,
    const char * path
) : m_editor_context{&editor_context},
    m_parent_id{parent_id},
    m_prefab{path} {

}

bool ActionInstantiatePrefab::apply() {
    m_node_id = m_prefab.instantiate(m_editor_context->scene(), m_parent_id);
    return true;
}

bool ActionInstantiatePrefab::unapply() {
    Scene & scene = m_editor_context->scene();
    scene.remove_node(m_node_id);
    return true;
}
